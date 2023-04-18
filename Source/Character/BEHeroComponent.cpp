// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BEHeroComponent.h"

#include "Character/BEPawnExtensionComponent.h"
#include "Character/BEPawnData.h"
#include "Character/BECharacter.h"
#include "Player/BEPlayerController.h"
#include "Player/BEPlayerState.h"
#include "Ability/BEAbilitySystemComponent.h"
#include "Input/BEInputConfig.h"
#include "Input/BEInputComponent.h"
#include "Camera/BECameraComponent.h"
#include "Settings/BESettingsLocal.h"
#include "System/BEAssetManager.h"
#include "BELogChannels.h"
#include "BEGameplayTags.h"

#include "GameFramework/Pawn.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Components/GameFrameworkComponentManager.h"
#include "PlayerMappableInputConfig.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEHeroComponent)

#if WITH_EDITOR
#include "Misc/UObjectToken.h"
#endif	// WITH_EDITOR

namespace BEHero
{
	static const float LookYawRate = 300.0f;
	static const float LookPitchRate = 165.0f;
};

const FName UBEHeroComponent::NAME_BindInputsNow("BindInputsNow");
const FName UBEHeroComponent::NAME_ActorFeatureName("Hero");


UBEHeroComponent::UBEHeroComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilityCameraMode = nullptr;
	bReadyToBindInputs = false;
}

void UBEHeroComponent::OnRegister()
{
	Super::OnRegister();

	if (!GetPawn<APawn>())
	{
		UE_LOG(LogBE, Error, TEXT("[UBEHeroComponent::OnRegister] This component has been added to a blueprint whose base class is not a Pawn. To use this component, it MUST be placed on a Pawn Blueprint."));

#if WITH_EDITOR
		if (GIsEditor)
		{
			static const FText Message = NSLOCTEXT("BEHeroComponent", "NotOnPawnError", "has been added to a blueprint whose base class is not a Pawn. To use this component, it MUST be placed on a Pawn Blueprint. This will cause a crash if you PIE!");
			static const FName HeroMessageLogName = TEXT("BEHeroComponent");

			FMessageLog(HeroMessageLogName).Error()
				->AddToken(FUObjectToken::Create(this, FText::FromString(GetNameSafe(this))))
				->AddToken(FTextToken::Create(Message));

			FMessageLog(HeroMessageLogName).Open();
		}
#endif
	}
	else
	{
		// Register with the init state system early, this will only work if this is a game world
		RegisterInitStateFeature();
	}
}

void UBEHeroComponent::BeginPlay()
{
	Super::BeginPlay();

	// Listen for when the pawn extension component changes init state
	BindOnActorInitStateChanged(UBEPawnExtensionComponent::NAME_ActorFeatureName, FGameplayTag(), false);

	// Notifies that we are done spawning, then try the rest of initialization
	ensure(TryToChangeInitState(TAG_InitState_Spawned));
	CheckDefaultInitialization();
}

void UBEHeroComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
}

void UBEHeroComponent::InitializePlayerInput(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	const APlayerController* PC = GetController<APlayerController>();
	check(PC);

	const ULocalPlayer* LP = PC->GetLocalPlayer();
	check(LP);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(Subsystem);

	Subsystem->ClearAllMappings();

	if (const UBEPawnExtensionComponent* PawnExtComp = UBEPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		if (const UBEPawnData* PawnData = PawnExtComp->GetPawnData<UBEPawnData>())
		{
			if (const UBEInputConfig* InputConfig = PawnData->InputConfig)
			{
				// Register any default input configs with the settings so that they will be applied to the player during AddInputMappings
				for (const FMappableConfigPair& Pair : DefaultInputConfigs)
				{
					if (Pair.bShouldActivateAutomatically && Pair.CanBeActivated())
					{
						FModifyContextOptions Options = {};
						Options.bIgnoreAllPressedKeysUntilRelease = false;
						// Actually add the config to the local player							
						Subsystem->AddPlayerMappableConfig(Pair.Config.LoadSynchronous(), Options);
					}
				}
				
				UBEInputComponent* BEIC = CastChecked<UBEInputComponent>(PlayerInputComponent);
				BEIC->AddInputMappings(InputConfig, Subsystem);

				TArray<uint32> BindHandles;
				BEIC->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, /*out*/ BindHandles);

				BEIC->BindNativeAction(InputConfig, TAG_Input_Move_KM, ETriggerEvent::Triggered, this, &ThisClass::Input_Move, /*bLogIfNotFound=*/ false);
				BEIC->BindNativeAction(InputConfig, TAG_Input_Move_Pad, ETriggerEvent::Triggered, this, &ThisClass::Input_Move, /*bLogIfNotFound=*/ false);
				BEIC->BindNativeAction(InputConfig, TAG_Input_Look_KM, ETriggerEvent::Triggered, this, &ThisClass::Input_LookMouse, /*bLogIfNotFound=*/ false);
				BEIC->BindNativeAction(InputConfig, TAG_Input_Look_Pad, ETriggerEvent::Triggered, this, &ThisClass::Input_LookStick, /*bLogIfNotFound=*/ false);
			}
		}
	}

	if (ensure(!bReadyToBindInputs))
	{
		bReadyToBindInputs = true;
	}

	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(const_cast<APlayerController*>(PC), NAME_BindInputsNow);
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(const_cast<APawn*>(Pawn), NAME_BindInputsNow);
}

bool UBEHeroComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);

	APawn* Pawn = GetPawn<APawn>();

	if (!CurrentState.IsValid() && DesiredState == TAG_InitState_Spawned)
	{
		// As long as we have a real pawn, let us transition
		if (Pawn)
		{
			return true;
		}
	}
	else if (CurrentState == TAG_InitState_Spawned && DesiredState == TAG_InitState_DataAvailable)
	{
		// The player state is required.
		if (!GetPlayerState<ABEPlayerState>())
		{
			return false;
		}

		// If we're authority or autonomous, we need to wait for a controller with registered ownership of the player state.
		if (Pawn->GetLocalRole() != ROLE_SimulatedProxy)
		{
			AController* Controller = GetController<AController>();

			const bool bHasControllerPairedWithPS = (Controller != nullptr) && \
				(Controller->PlayerState != nullptr) && \
				(Controller->PlayerState->GetOwner() == Controller);

			if (!bHasControllerPairedWithPS)
			{
				return false;
			}
		}

		const bool bIsLocallyControlled = Pawn->IsLocallyControlled();
		const bool bIsBot = Pawn->IsBotControlled();

		if (bIsLocallyControlled && !bIsBot)
		{
			ABEPlayerController* BEPC = GetController<ABEPlayerController>();

			// The input component and local player is required when locally controlled.
			if (!Pawn->InputComponent || !BEPC || !BEPC->GetLocalPlayer())
			{
				return false;
			}
		}

		return true;
	}
	else if (CurrentState == TAG_InitState_DataAvailable && DesiredState == TAG_InitState_DataInitialized)
	{
		// Wait for player state and extension component
		ABEPlayerState* BEPS = GetPlayerState<ABEPlayerState>();

		return BEPS && Manager->HasFeatureReachedInitState(Pawn, UBEPawnExtensionComponent::NAME_ActorFeatureName, TAG_InitState_DataInitialized);
	}
	else if (CurrentState == TAG_InitState_DataInitialized && DesiredState == TAG_InitState_GameplayReady)
	{
		// TODO add ability initialization checks?
		return true;
	}

	return false;
}

void UBEHeroComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	if (CurrentState == TAG_InitState_DataAvailable && DesiredState == TAG_InitState_DataInitialized)
	{
		APawn* Pawn = GetPawn<APawn>();
		ABEPlayerState* BEPS = GetPlayerState<ABEPlayerState>();
		if (!ensure(Pawn && BEPS))
		{
			return;
		}

		const bool bIsLocallyControlled = Pawn->IsLocallyControlled();
		const UBEPawnData* PawnData = nullptr;

		if (UBEPawnExtensionComponent* PawnExtComp = UBEPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
		{
			PawnData = PawnExtComp->GetPawnData<UBEPawnData>();

			// The player state holds the persistent data for this player (state that persists across deaths and multiple pawns).
			// The ability system component and attribute sets live on the player state.
			PawnExtComp->InitializeAbilitySystem(BEPS->GetBEAbilitySystemComponent(), BEPS);
		}

		if (ABEPlayerController* BEPC = GetController<ABEPlayerController>())
		{
			if (Pawn->InputComponent != nullptr)
			{
				InitializePlayerInput(Pawn->InputComponent);
			}
		}

		if (bIsLocallyControlled && PawnData)
		{
			if (UBECameraComponent* CameraComponent = UBECameraComponent::FindCameraComponent(Pawn))
			{
				CameraComponent->DetermineCameraModeDelegate.BindUObject(this, &ThisClass::DetermineCameraMode);
			}
		}
	}
}

void UBEHeroComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	if (Params.FeatureName == UBEPawnExtensionComponent::NAME_ActorFeatureName)
	{
		if (Params.FeatureState == TAG_InitState_DataInitialized)
		{
			// If the extension component says all all other components are initialized, try to progress to next state
			CheckDefaultInitialization();
		}
	}
}

void UBEHeroComponent::CheckDefaultInitialization()
{
	static const TArray<FGameplayTag> StateChain = { TAG_InitState_Spawned, TAG_InitState_DataAvailable, TAG_InitState_DataInitialized, TAG_InitState_GameplayReady };

	// This will try to progress from spawned (which is only set in BeginPlay) through the data initialization stages until it gets to gameplay ready
	ContinueInitStateChain(StateChain);
}

void UBEHeroComponent::AddAdditionalInputConfig(const UBEInputConfig* InputConfig)
{
	TArray<uint32> BindHandles;

	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	UBEInputComponent* BEIC = Pawn->FindComponentByClass<UBEInputComponent>();
	check(BEIC);

	const APlayerController* PC = GetController<APlayerController>();
	check(PC);

	const ULocalPlayer* LP = PC->GetLocalPlayer();
	check(LP);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(Subsystem);

	if (const UBEPawnExtensionComponent* PawnExtComp = UBEPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		BEIC->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, /*out*/ BindHandles);
	}
}

void UBEHeroComponent::RemoveAdditionalInputConfig(const UBEInputConfig* InputConfig)
{
	//@TODO: Implement me!
}

bool UBEHeroComponent::IsReadyToBindInputs() const
{
	return bReadyToBindInputs;
}

void UBEHeroComponent::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (const APawn* Pawn = GetPawn<APawn>())
	{
		if (const UBEPawnExtensionComponent* PawnExtComp = UBEPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
		{
			if (UBEAbilitySystemComponent* BEASC = PawnExtComp->GetBEAbilitySystemComponent())
			{
				BEASC->AbilityInputTagPressed(InputTag);
			}
		}	
	}
}

void UBEHeroComponent::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	if (const UBEPawnExtensionComponent* PawnExtComp = UBEPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		if (UBEAbilitySystemComponent* BEASC = PawnExtComp->GetBEAbilitySystemComponent())
		{
			BEASC->AbilityInputTagReleased(InputTag);
		}
	}
}

void UBEHeroComponent::Input_Move(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();
	AController* Controller = Pawn ? Pawn->GetController() : nullptr;
	
	if (Controller)
	{
		const FVector2D Value = InputActionValue.Get<FVector2D>();
		const FRotator MovementRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

		if (Value.X != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
			Pawn->AddMovementInput(MovementDirection, Value.X);
		}

		if (Value.Y != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
			Pawn->AddMovementInput(MovementDirection, Value.Y);
		}
	}
}

void UBEHeroComponent::Input_LookMouse(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();

	if (!Pawn)
	{
		return;
	}
	
	const FVector2D Value = InputActionValue.Get<FVector2D>();

	if (Value.X != 0.0f)
	{
		Pawn->AddControllerYawInput(Value.X);
	}

	if (Value.Y != 0.0f)
	{
		Pawn->AddControllerPitchInput(Value.Y);
	}
}

void UBEHeroComponent::Input_LookStick(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();

	if (!Pawn)
	{
		return;
	}
	
	const FVector2D Value = InputActionValue.Get<FVector2D>();

	const UWorld* World = GetWorld();
	check(World);

	if (Value.X != 0.0f)
	{
		Pawn->AddControllerYawInput(Value.X * BEHero::LookYawRate * World->GetDeltaSeconds());
	}

	if (Value.Y != 0.0f)
	{
		Pawn->AddControllerPitchInput(Value.Y * BEHero::LookPitchRate * World->GetDeltaSeconds());
	}
}

TSubclassOf<UBECameraMode> UBEHeroComponent::DetermineCameraMode() const
{
	if (AbilityCameraMode)
	{
		return AbilityCameraMode;
	}

	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return nullptr;
	}

	if (UBEPawnExtensionComponent* PawnExtComp = UBEPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		if (const UBEPawnData* PawnData = PawnExtComp->GetPawnData<UBEPawnData>())
		{
			return PawnData->DefaultCameraMode;
		}
	}

	return nullptr;
}

void UBEHeroComponent::SetAbilityCameraMode(TSubclassOf<UBECameraMode> CameraMode, const FGameplayAbilitySpecHandle& OwningSpecHandle)
{
	if (CameraMode)
	{
		AbilityCameraMode = CameraMode;
		AbilityCameraModeOwningSpecHandle = OwningSpecHandle;
	}
}

void UBEHeroComponent::ClearAbilityCameraMode(const FGameplayAbilitySpecHandle& OwningSpecHandle)
{
	if (AbilityCameraModeOwningSpecHandle == OwningSpecHandle)
	{
		AbilityCameraMode = nullptr;
		AbilityCameraModeOwningSpecHandle = FGameplayAbilitySpecHandle();
	}
}

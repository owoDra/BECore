// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BECharacterPlayableComponent.h"

#include "Character/Component/BECharacterBasicComponent.h"
#include "Character/BECharacterData.h"
#include "Character/BECharacter.h"
#include "Player/BEPlayerController.h"
#include "Player/BEPlayerState.h"
#include "Ability/BEAbilitySystemComponent.h"
#include "Input/BEInputConfig.h"
#include "Input/BEInputComponent.h"
#include "Setting/BESettingsLocal.h"
#include "System/BEAssetManager.h"
#include "BELogChannels.h"
#include "BEGameplayTags.h"

#include "GameFramework/Pawn.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Components/GameFrameworkComponentManager.h"
#include "PlayerMappableInputConfig.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BECharacterPlayableComponent)

namespace BEPlayable
{
	static const float LookYawRate = 300.0f;
	static const float LookPitchRate = 165.0f;
};

const FName UBECharacterPlayableComponent::NAME_BindInputsNow("BindInputsNow");
const FName UBECharacterPlayableComponent::NAME_ActorFeatureName("CharacterPlayable");


UBECharacterPlayableComponent::UBECharacterPlayableComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReadyToBindInputs = false;
}


void UBECharacterPlayableComponent::OnRegister()
{
	Super::OnRegister();

	const APawn* Pawn = GetPawn<APawn>();
	ensureAlwaysMsgf((Pawn != nullptr), TEXT("BECharacterPlayableComponent on [%s] can only be added to Pawn actors."), *GetNameSafe(GetOwner()));

	TArray<UActorComponent*> Components;
	Pawn->GetComponents(UBECharacterPlayableComponent::StaticClass(), Components);
	ensureAlwaysMsgf((Components.Num() == 1), TEXT("Only one BECharacterPlayableComponent should exist on [%s]."), *GetNameSafe(GetOwner()));

	// �R���|�[�l���g�� GameWorld �ɑ��݂���ۂɁ@InitStateSystem �ɓo�^����
	RegisterInitStateFeature();
}

void UBECharacterPlayableComponent::BeginPlay()
{
	Super::BeginPlay();

	// ���ׂĂ� Feature �ւ̕ύX�����b�X������
	BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);

	// InisStateSystem �ɂ��̃R���|�[�l���g���X�|�[���������Ƃ�m�点��B
	ensure(TryToChangeInitState(TAG_InitState_Spawned));

	// �c��̏��������s��
	CheckDefaultInitialization();
}

void UBECharacterPlayableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
}


bool UBECharacterPlayableComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);

	APawn* Pawn = GetPawn<APawn>();

	/**
	 * [InitState None] -> [InitState Spawned]
	 *
	 *  Pawn ���L��
	 */
	if (!CurrentState.IsValid() && DesiredState == TAG_InitState_Spawned)
	{
		if (Pawn != nullptr)
		{
			return true;
		}
	}

	/**
	 * [InitState Spawned] -> [InitState DataAvailable]
	 *
	 *  Controller ���L��
	 *  PlayerState ���L��
	 *  InputComponent ���L��
	 *  Bot �ł͂Ȃ�
	 */
	else if (CurrentState == TAG_InitState_Spawned && DesiredState == TAG_InitState_DataAvailable)
	{
		// �L���� PlayerState �����݂��邩
		if (GetPlayerState<ABEPlayerState>() == nullptr)
		{
			return false;
		}

		// �V�~�����[�V�����łȂ��A���� Controller �� PlayerState �����L���Ă��邩
		if (Pawn->GetLocalRole() != ROLE_SimulatedProxy)
		{
			AController* Controller = GetController<AController>();
			if (Controller == nullptr)
			{
				return false;
			}

			if (Controller->PlayerState == nullptr)
			{
				return false;
			}

			if (Controller->PlayerState->GetOwner() != Controller)
			{
				return false;
			}
		}

		// �{�b�g�ł͂Ȃ����[�J���R���g���[���[�����L���邩�ǂ���
		const bool bIsLocallyControlled = Pawn->IsLocallyControlled();
		const bool bIsBot = Pawn->IsBotControlled();
		if (bIsLocallyControlled && !bIsBot)
		{
			ABEPlayerController* BEPC = GetController<ABEPlayerController>();

			if (!Pawn->InputComponent || !BEPC || !BEPC->GetLocalPlayer())
			{
				return false;
			}
		}

		return true;
	}

	/**
	 * [InitState DataAvailable] -> [InitState DataInitialized]
	 *
	 *  ���̂��ׂĂ� Feature �� DataAvailable �ɓ��B���Ă���
	 */
	else if (CurrentState == TAG_InitState_DataAvailable && DesiredState == TAG_InitState_DataInitialized)
	{
		// CharacterBasicComponent �� DataInitialized �ɓ��B���Ă��邩
		// �܂�A���̂��ׂĂ� Feature �� DataAvailable �ɓ��B���Ă��邩
		return Manager->HasFeatureReachedInitState(Pawn, UBECharacterBasicComponent::NAME_ActorFeatureName, TAG_InitState_DataInitialized);
	}

	/**
	 * [InitState DataInitialized] -> [InitState GameplayReady]
	 *
	 *  �������ŋ���
	 */
	else if (CurrentState == TAG_InitState_DataInitialized && DesiredState == TAG_InitState_GameplayReady)
	{
		return true;
	}

	return false;
}

void UBECharacterPlayableComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	/**
	 * [InitState DataAvailable] -> [InitState DataInitialized]
	 *
	 *  InputComponent ��������
	 */
	if (CurrentState == TAG_InitState_DataAvailable && DesiredState == TAG_InitState_DataInitialized)
	{
		APawn* Pawn = GetPawn<APawn>();
		ABEPlayerState* BEPS = GetPlayerState<ABEPlayerState>();
		if (!ensure(Pawn && BEPS))
		{
			return;
		}

		const bool bIsLocallyControlled = Pawn->IsLocallyControlled();
		const UBECharacterData* CharacterData = nullptr;

		if (ABEPlayerController* BEPC = GetController<ABEPlayerController>())
		{
			if (Pawn->InputComponent != nullptr)
			{
				InitializePlayerInput(Pawn->InputComponent);
			}
		}
	}
}

void UBECharacterPlayableComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	// CharacterBasicComponent �� DataInitialized �ɓ��B���Ă��邩
	// �܂�A���̂��ׂĂ� Feature �� DataAvailable �ɓ��B���Ă��邩
	if (Params.FeatureName == UBECharacterBasicComponent::NAME_ActorFeatureName)
	{
		if (Params.FeatureState == TAG_InitState_DataInitialized)
		{
			CheckDefaultInitialization();
		}
	}
}

void UBECharacterPlayableComponent::CheckDefaultInitialization()
{
	static const TArray<FGameplayTag> StateChain = {
		TAG_InitState_Spawned,
		TAG_InitState_DataAvailable,
		TAG_InitState_DataInitialized,
		TAG_InitState_GameplayReady 
	};

	ContinueInitStateChain(StateChain);
}


void UBECharacterPlayableComponent::InitializePlayerInput(UInputComponent* PlayerInputComponent)
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

	if (const UBECharacterBasicComponent* CharacterBasic = UBECharacterBasicComponent::FindCharacterBasicComponent(Pawn))
	{
		if (const UBECharacterData* CharacterData = CharacterBasic->GetCharacterData<UBECharacterData>())
		{
			if (const UBEInputConfig* InputConfig = CharacterData->InputConfig)
			{
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

void UBECharacterPlayableComponent::AddAdditionalInputConfig(const UBEInputConfig* InputConfig)
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

	if (const UBECharacterBasicComponent* CharacterBasic = UBECharacterBasicComponent::FindCharacterBasicComponent(Pawn))
	{
		BEIC->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, /*out*/ BindHandles);
	}
}

void UBECharacterPlayableComponent::RemoveAdditionalInputConfig(const UBEInputConfig* InputConfig)
{
	
}

bool UBECharacterPlayableComponent::IsReadyToBindInputs() const
{
	return bReadyToBindInputs;
}

void UBECharacterPlayableComponent::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (const APawn* Pawn = GetPawn<APawn>())
	{
		if (const UBECharacterBasicComponent* CharacterBasic = UBECharacterBasicComponent::FindCharacterBasicComponent(Pawn))
		{
			if (UBEAbilitySystemComponent* BEASC = CharacterBasic->GetBEAbilitySystemComponent())
			{
				BEASC->AbilityInputTagPressed(InputTag);
			}
		}	
	}
}

void UBECharacterPlayableComponent::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	if (const UBECharacterBasicComponent* CharacterBasic = UBECharacterBasicComponent::FindCharacterBasicComponent(Pawn))
	{
		if (UBEAbilitySystemComponent* BEASC = CharacterBasic->GetBEAbilitySystemComponent())
		{
			BEASC->AbilityInputTagReleased(InputTag);
		}
	}
}

void UBECharacterPlayableComponent::Input_Move(const FInputActionValue& InputActionValue)
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

void UBECharacterPlayableComponent::Input_LookMouse(const FInputActionValue& InputActionValue)
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

void UBECharacterPlayableComponent::Input_LookStick(const FInputActionValue& InputActionValue)
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
		Pawn->AddControllerYawInput(Value.X * BEPlayable::LookYawRate * World->GetDeltaSeconds());
	}

	if (Value.Y != 0.0f)
	{
		Pawn->AddControllerPitchInput(Value.Y * BEPlayable::LookPitchRate * World->GetDeltaSeconds());
	}
}

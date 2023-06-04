// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BEPawnPlayableComponent.h"

#include "Character/Component/BEPawnBasicComponent.h"
#include "Character/BEPawnData.h"
#include "Character/BECharacter.h"
#include "Player/BEPlayerController.h"
#include "Player/BEPlayerState.h"
#include "Ability/BEAbilitySystemComponent.h"
#include "Input/BEInputConfig.h"
#include "Input/BEInputComponent.h"
#include "GameSetting/BEGameDeviceSettings.h"
#include "System/BEAssetManager.h"
#include "BELogChannels.h"
#include "GameplayTag/BETags_Input.h"
#include "GameplayTag/BETags_InitState.h"

#include "GameFramework/Pawn.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Components/GameFrameworkComponentManager.h"
#include "PlayerMappableInputConfig.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEPawnPlayableComponent)

namespace BEPlayable
{
	static const float LookYawRate = 300.0f;
	static const float LookPitchRate = 165.0f;
};

const FName UBEPawnPlayableComponent::NAME_BindInputsNow("BindInputsNow");
const FName UBEPawnPlayableComponent::NAME_ActorFeatureName("CharacterPlayable");


UBEPawnPlayableComponent::UBEPawnPlayableComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReadyToBindInputs = false;
}


void UBEPawnPlayableComponent::OnRegister()
{
	Super::OnRegister();

	const APawn* Pawn = GetPawn<APawn>();
	ensureAlwaysMsgf((Pawn != nullptr), TEXT("BEPawnPlayableComponent on [%s] can only be added to Pawn actors."), *GetNameSafe(GetOwner()));

	TArray<UActorComponent*> Components;
	Pawn->GetComponents(UBEPawnPlayableComponent::StaticClass(), Components);
	ensureAlwaysMsgf((Components.Num() == 1), TEXT("Only one BEPawnPlayableComponent should exist on [%s]."), *GetNameSafe(GetOwner()));

	// �R���|�[�l���g�� GameWorld �ɑ��݂���ۂɁ@InitStateSystem �ɓo�^����
	RegisterInitStateFeature();
}

void UBEPawnPlayableComponent::BeginPlay()
{
	Super::BeginPlay();

	// ���ׂĂ� Feature �ւ̕ύX�����b�X������
	BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);

	// InisStateSystem �ɂ��̃R���|�[�l���g���X�|�[���������Ƃ�m�点��B
	ensure(TryToChangeInitState(TAG_InitState_Spawned));

	// �c��̏��������s��
	CheckDefaultInitialization();
}

void UBEPawnPlayableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
}


bool UBEPawnPlayableComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
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
		return Manager->HasFeatureReachedInitState(Pawn, UBEPawnBasicComponent::NAME_ActorFeatureName, TAG_InitState_DataInitialized);
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

void UBEPawnPlayableComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
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
		const UBEPawnData* PawnData = nullptr;

		if (ABEPlayerController* BEPC = GetController<ABEPlayerController>())
		{
			if (Pawn->InputComponent != nullptr)
			{
				InitializePlayerInput(Pawn->InputComponent);
			}
		}
	}
}

void UBEPawnPlayableComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	// CharacterBasicComponent �� DataInitialized �ɓ��B���Ă��邩
	// �܂�A���̂��ׂĂ� Feature �� DataAvailable �ɓ��B���Ă��邩
	if (Params.FeatureName == UBEPawnBasicComponent::NAME_ActorFeatureName)
	{
		if (Params.FeatureState == TAG_InitState_DataInitialized)
		{
			CheckDefaultInitialization();
		}
	}
}

void UBEPawnPlayableComponent::CheckDefaultInitialization()
{
	static const TArray<FGameplayTag> StateChain = {
		TAG_InitState_Spawned,
		TAG_InitState_DataAvailable,
		TAG_InitState_DataInitialized,
		TAG_InitState_GameplayReady 
	};

	ContinueInitStateChain(StateChain);
}


void UBEPawnPlayableComponent::InitializePlayerInput(UInputComponent* PlayerInputComponent)
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

	if (const UBEPawnBasicComponent* CharacterBasic = UBEPawnBasicComponent::FindPawnBasicComponent(Pawn))
	{
		if (const UBEPawnData* PawnData = CharacterBasic->GetPawnData())
		{
			if (const UBEInputConfig* InputConfig = PawnData->InputConfig)
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

void UBEPawnPlayableComponent::AddAdditionalInputConfig(const UBEInputConfig* InputConfig)
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

	if (const UBEPawnBasicComponent* CharacterBasic = UBEPawnBasicComponent::FindPawnBasicComponent(Pawn))
	{
		BEIC->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, /*out*/ BindHandles);
	}
}

void UBEPawnPlayableComponent::RemoveAdditionalInputConfig(const UBEInputConfig* InputConfig)
{
	
}

bool UBEPawnPlayableComponent::IsReadyToBindInputs() const
{
	return bReadyToBindInputs;
}

void UBEPawnPlayableComponent::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (const APawn* Pawn = GetPawn<APawn>())
	{
		if (const UBEPawnBasicComponent* CharacterBasic = UBEPawnBasicComponent::FindPawnBasicComponent(Pawn))
		{
			if (UBEAbilitySystemComponent* BEASC = CharacterBasic->GetBEAbilitySystemComponent())
			{
				BEASC->AbilityInputTagPressed(InputTag);
			}
		}	
	}
}

void UBEPawnPlayableComponent::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	if (const UBEPawnBasicComponent* CharacterBasic = UBEPawnBasicComponent::FindPawnBasicComponent(Pawn))
	{
		if (UBEAbilitySystemComponent* BEASC = CharacterBasic->GetBEAbilitySystemComponent())
		{
			BEASC->AbilityInputTagReleased(InputTag);
		}
	}
}

void UBEPawnPlayableComponent::Input_Move(const FInputActionValue& InputActionValue)
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

void UBEPawnPlayableComponent::Input_LookMouse(const FInputActionValue& InputActionValue)
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

void UBEPawnPlayableComponent::Input_LookStick(const FInputActionValue& InputActionValue)
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

UBEPawnPlayableComponent* UBEPawnPlayableComponent::FindCharacterPlayableComponent(const APawn* Pawn)
{
	return (Pawn ? Pawn->FindComponentByClass<UBEPawnPlayableComponent>() : nullptr);
}

// Copyright Eigi Chin

#include "BEPawnBasicComponent.h"

#include "Ability/BEAbilitySystemComponent.h"
#include "Character/BEPawnData.h"
#include "Player/BEPlayerState.h"
#include "BELogChannels.h"
#include "GameplayTag/BETags_InitState.h"
#include "GameplayTag/BETags_GameplayAbility.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Containers/Array.h"
#include "Containers/Set.h"
#include "Containers/UnrealString.h"
#include "Engine/EngineBaseTypes.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameplayTagContainer.h"
#include "HAL/Platform.h"
#include "Logging/LogCategory.h"
#include "Logging/LogMacros.h"
#include "Misc/AssertionMacros.h"
#include "Net/UnrealNetwork.h"
#include "Templates/SharedPointer.h"
#include "Trace/Detail/Channel.h"
#include "UObject/UObjectBaseUtility.h"
#include "UObject/UnrealNames.h"
#include "UObject/WeakObjectPtr.h"
#include "UObject/WeakObjectPtrTemplates.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEPawnBasicComponent)

const FName UBEPawnBasicComponent::NAME_ActorFeatureName("CharacterBasic");


UBEPawnBasicComponent::UBEPawnBasicComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);

	PawnData = nullptr;
	AbilitySystemComponent = nullptr;
}

void UBEPawnBasicComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UBEPawnBasicComponent, PawnData);
}


void UBEPawnBasicComponent::OnRegister()
{
	Super::OnRegister();

	const APawn* Pawn = GetPawn<APawn>();
	ensureAlwaysMsgf((Pawn != nullptr), TEXT("BEPawnBasicComponent on [%s] can only be added to Pawn actors."), *GetNameSafe(GetOwner()));

	TArray<UActorComponent*> Components;
	Pawn->GetComponents(UBEPawnBasicComponent::StaticClass(), Components);
	ensureAlwaysMsgf((Components.Num() == 1), TEXT("Only one BEPawnBasicComponent should exist on [%s]."), *GetNameSafe(GetOwner()));

	// �R���|�[�l���g�� GameWorld �ɑ��݂���ۂɁ@InitStateSystem �ɓo�^����
	RegisterInitStateFeature();
}

void UBEPawnBasicComponent::BeginPlay()
{
	Super::BeginPlay();

	// ���ׂĂ� Feature �ւ̕ύX�����b�X������
	BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);

	// InisStateSystem �ɂ��̃R���|�[�l���g���X�|�[���������Ƃ�m�点��B
	ensure(TryToChangeInitState(TAG_InitState_Spawned));

	// �c��̏��������s��
	CheckDefaultInitialization();
}

void UBEPawnBasicComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UninitializeAbilitySystem();
	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
}


bool UBEPawnBasicComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
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
	 *  Pawn ���L��
	 *  AbilitySystemComponent ���L��
	 *  ���F���ꂽ���[�J���R���g���[���[������
	 */
	if (CurrentState == TAG_InitState_Spawned && DesiredState == TAG_InitState_DataAvailable)
	{
		// �L���� PawnData �����݂��邩
		if (PawnData == nullptr)
		{
			return false;
		}

		// ���F���ꂽ���[�J���R���g���[���[�����邩
		const bool bHasAuthority = Pawn->HasAuthority();
		const bool bIsLocallyControlled = Pawn->IsLocallyControlled();
		if (bHasAuthority || bIsLocallyControlled)
		{
			AController* Controller = GetController<AController>();
			if (Controller == nullptr)
			{
				return false;
			}
		}

		return true;
	}

	/**
	 * [InitState DataAvailable] -> [InitState DataInitialized]
	 *
	 *  ���ׂĂ� Feature �� DataAvailable �ɓ��B���Ă���
	 */
	else if (CurrentState == TAG_InitState_DataAvailable && DesiredState == TAG_InitState_DataInitialized)
	{
		// ���ׂĂ� Feature �� DataAvailable �ɓ��B���Ă��邩
		return Manager->HaveAllFeaturesReachedInitState(Pawn, TAG_InitState_DataAvailable);
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

void UBEPawnBasicComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	/**
	 * [InitState DataAvailable] -> [InitState DataInitialized]
	 *
	 *  PawnData �� AbilitySystem �̏�����
	 */
	if (CurrentState == TAG_InitState_DataAvailable && DesiredState == TAG_InitState_DataInitialized)
	{
		APawn* Pawn = GetPawn<APawn>();
		if (!ensure(Pawn))
		{
			return;
		}

		ABEPlayerState* BEPS = GetPlayerState<ABEPlayerState>();
		if (ensure(BEPS))
		{
			InitializeAbilitySystem(BEPS->GetBEAbilitySystemComponent(), BEPS);
		}

		check(AbilitySystemComponent);
		check(PawnData);

		if (Pawn->HasAuthority())
		{
			AbilitySystemComponent->SetTagRelationshipMapping(PawnData->TagRelationshipMapping);
		}

		OnPawnInitialized.Broadcast();
	}
}

void UBEPawnBasicComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	// ���� Feature �� DataAvailable �ɂ���Ƃ��ADataInitialized �ɑJ�ڂ���K�v�����邩�ǂ������m�F����
	if (Params.FeatureName != NAME_ActorFeatureName)
	{
		if (Params.FeatureState == TAG_InitState_DataAvailable)
		{
			CheckDefaultInitialization();
		}
	}
}

void UBEPawnBasicComponent::CheckDefaultInitialization()
{
	// �������󋵂��m�F����O�ɁA�ˑ��֌W�ɂ���\������ Feature �̏��������݂�
	CheckDefaultInitializationForImplementers();

	static const TArray<FGameplayTag> StateChain = {
		TAG_InitState_Spawned,
		TAG_InitState_DataAvailable,
		TAG_InitState_DataInitialized,
		TAG_InitState_GameplayReady 
	};

	ContinueInitStateChain(StateChain);
}


void UBEPawnBasicComponent::OnRep_PawnData()
{
	CheckDefaultInitialization();
}

void UBEPawnBasicComponent::SetPawnData(const UBEPawnData* InPawnData)
{
	check(InPawnData);

	APawn* Pawn = GetPawnChecked<APawn>();
	if (!Pawn->HasAuthority())
	{
		return;
	}

	// ���� PawnData ���o�^�ς݂̏ꍇ�̓L�����Z���B
	// PawnData �̓X�|�[�����܂��̓C���X�^���X�ł̂ݐݒ�\���ύX�s�\

	if (PawnData)
	{
		UE_LOG(LogBE, Error, TEXT("Trying to set PawnData [%s] on pawn [%s] that already has valid PawnData [%s]."), *GetNameSafe(InPawnData), *GetNameSafe(Pawn), *GetNameSafe(PawnData));
		return;
	}

	PawnData = InPawnData;

	Pawn->ForceNetUpdate();

	CheckDefaultInitialization();
}


void UBEPawnBasicComponent::HandleControllerChanged()
{
	if (AbilitySystemComponent && (AbilitySystemComponent->GetAvatarActor() == GetPawnChecked<APawn>()))
	{
		ensure(AbilitySystemComponent->AbilityActorInfo->OwnerActor == AbilitySystemComponent->GetOwnerActor());
		if (AbilitySystemComponent->GetOwnerActor() == nullptr)
		{
			UninitializeAbilitySystem();
		}
		else
		{
			AbilitySystemComponent->RefreshAbilityActorInfo();
		}
	}

	CheckDefaultInitialization();
}

void UBEPawnBasicComponent::HandlePlayerStateReplicated()
{
	if (ABEPlayerState* BEPS = GetPlayerState<ABEPlayerState>())
	{
		InitializeAbilitySystem(BEPS->GetBEAbilitySystemComponent(), BEPS);
	}
}

void UBEPawnBasicComponent::HandlePlayerInputComponentSetup()
{
	CheckDefaultInitialization();
}


void UBEPawnBasicComponent::InitializeAbilitySystem(UBEAbilitySystemComponent* InASC, AActor* InOwnerActor)
{
	check(InASC);
	check(InOwnerActor);

	if (AbilitySystemComponent == InASC)
	{
		// The ability system component hasn't changed.
		return;
	}

	if (AbilitySystemComponent)
	{
		// Clean up the old ability system component.
		UninitializeAbilitySystem();
	}

	APawn* Pawn = GetPawnChecked<APawn>();
	APawn* ExistingAvatar = Cast<APawn>(InASC->GetAvatarActor());

	UE_LOG(LogBE, Verbose, TEXT("Setting up ASC [%s] on pawn [%s] owner [%s], existing [%s] "), *GetNameSafe(InASC), *GetNameSafe(Pawn), *GetNameSafe(InOwnerActor), *GetNameSafe(ExistingAvatar));

	if ((ExistingAvatar != nullptr) && (ExistingAvatar != Pawn))
	{
		UE_LOG(LogBE, Log, TEXT("Existing avatar (authority=%d)"), ExistingAvatar->HasAuthority() ? 1 : 0);

		// There is already a pawn acting as the ASC's avatar, so we need to kick it out
		// This can happen on clients if they're lagged: their new pawn is spawned + possessed before the dead one is removed
		ensure(!ExistingAvatar->HasAuthority());

		if (UBEPawnBasicComponent* OtherExtensionComponent = FindPawnBasicComponent(ExistingAvatar))
		{
			OtherExtensionComponent->UninitializeAbilitySystem();
		}
	}

	AbilitySystemComponent = InASC;
	AbilitySystemComponent->InitAbilityActorInfo(InOwnerActor, Pawn);

	OnAbilitySystemInitialized.Broadcast();

	CheckDefaultInitialization();
}

void UBEPawnBasicComponent::UninitializeAbilitySystem()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	// Uninitialize the ASC if we're still the avatar actor (otherwise another pawn already did it when they became the avatar actor)
	if (AbilitySystemComponent->GetAvatarActor() == GetOwner())
	{
		FGameplayTagContainer AbilityTypesToIgnore;
		AbilityTypesToIgnore.AddTag(TAG_Ability_Behavior_SurvivesDeath);

		AbilitySystemComponent->CancelAbilities(nullptr, &AbilityTypesToIgnore);
		AbilitySystemComponent->ClearAbilityInput();
		AbilitySystemComponent->RemoveAllGameplayCues();

		if (AbilitySystemComponent->GetOwnerActor() != nullptr)
		{
			AbilitySystemComponent->SetAvatarActor(nullptr);
		}
		else
		{
			// If the ASC doesn't have a valid owner, we need to clear *all* actor info, not just the avatar pairing
			AbilitySystemComponent->ClearActorInfo();
		}

		OnAbilitySystemUninitialized.Broadcast();
	}

	AbilitySystemComponent = nullptr;
}


void UBEPawnBasicComponent::OnAbilitySystemInitialized_Register(FSimpleMulticastDelegate::FDelegate Delegate)
{
	if (!OnAbilitySystemInitialized.IsBoundToObject(Delegate.GetUObject()))
	{
		OnAbilitySystemInitialized.Add(Delegate);
	}
}

void UBEPawnBasicComponent::OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate Delegate)
{
	if (!OnAbilitySystemUninitialized.IsBoundToObject(Delegate.GetUObject()))
	{
		OnAbilitySystemUninitialized.Add(Delegate);
	}
}

void UBEPawnBasicComponent::OnPawnInitialized_Register(FSimpleMulticastDelegate::FDelegate Delegate)
{
	if (!OnPawnInitialized.IsBoundToObject(Delegate.GetUObject()))
	{
		OnPawnInitialized.Add(Delegate);
	}
}


UBEPawnBasicComponent* UBEPawnBasicComponent::FindPawnBasicComponent(const APawn* Pawn)
{
	return (Pawn ? Pawn->FindComponentByClass<UBEPawnBasicComponent>() : nullptr);
}

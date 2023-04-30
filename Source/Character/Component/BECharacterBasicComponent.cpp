// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BECharacterBasicComponent.h"

#include "Ability/BEAbilitySystemComponent.h"
#include "Character/BECharacterData.h"
#include "BELogChannels.h"
#include "BEGameplayTags.h"

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

#include UE_INLINE_GENERATED_CPP_BY_NAME(BECharacterBasicComponent)

class FLifetimeProperty;
class UActorComponent;

const FName UBECharacterBasicComponent::NAME_ActorFeatureName("CharacterBasic");


UBECharacterBasicComponent::UBECharacterBasicComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);

	CharacterData = nullptr;
	AbilitySystemComponent = nullptr;
}

void UBECharacterBasicComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UBECharacterBasicComponent, CharacterData);
}


void UBECharacterBasicComponent::OnRegister()
{
	Super::OnRegister();

	const APawn* Pawn = GetPawn<APawn>();
	ensureAlwaysMsgf((Pawn != nullptr), TEXT("BECharacterBasicComponent on [%s] can only be added to Pawn actors."), *GetNameSafe(GetOwner()));

	TArray<UActorComponent*> Components;
	Pawn->GetComponents(UBECharacterBasicComponent::StaticClass(), Components);
	ensureAlwaysMsgf((Components.Num() == 1), TEXT("Only one BECharacterBasicComponent should exist on [%s]."), *GetNameSafe(GetOwner()));

	// �R���|�[�l���g�� GameWorld �ɑ��݂���ۂɁ@InitStateSystem �ɓo�^����
	RegisterInitStateFeature();
}

void UBECharacterBasicComponent::BeginPlay()
{
	Super::BeginPlay();

	// ���ׂĂ� Feature �ւ̕ύX�����b�X������
	BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);

	// InisStateSystem �ɂ��̃R���|�[�l���g���X�|�[���������Ƃ�m�点��B
	ensure(TryToChangeInitState(TAG_InitState_Spawned));

	// �c��̏��������s��
	CheckDefaultInitialization();
}

void UBECharacterBasicComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UninitializeAbilitySystem();
	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
}


bool UBECharacterBasicComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
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
		// �L���� CharacterData �����݂��邩
		if (CharacterData == nullptr)
		{
			return false;
		}

		// �L���� AbilitySystemComponent �����݂��邩
		if (AbilitySystemComponent == nullptr)
		{
			return false;
		}

		// ���F���ꂽ���[�J���R���g���[���[�����邩
		const bool bHasAuthority = Pawn->HasAuthority();
		const bool bIsLocallyControlled = Pawn->IsLocallyControlled();
		if (bHasAuthority || bIsLocallyControlled)
		{
			if (GetController<AController>() == nullptr)
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

void UBECharacterBasicComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	// ���̃R���|�[�l���g�ł͉������Ȃ�
}

void UBECharacterBasicComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
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

void UBECharacterBasicComponent::CheckDefaultInitialization()
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


void UBECharacterBasicComponent::OnRep_CharacterData()
{
	CheckDefaultInitialization();
}

void UBECharacterBasicComponent::SetCharacterData(const UBECharacterData* InCharacterData)
{
	check(InCharacterData);

	APawn* Pawn = GetPawnChecked<APawn>();
	if (Pawn->GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	if (CharacterData)
	{
		UE_LOG(LogBE, Error, TEXT("Trying to set CharacterData [%s] on pawn [%s] that already has valid CharacterData [%s]."), *GetNameSafe(InCharacterData), *GetNameSafe(Pawn), *GetNameSafe(CharacterData));
		return;
	}

	CharacterData = InCharacterData;

	Pawn->ForceNetUpdate();

	CheckDefaultInitialization();
}


void UBECharacterBasicComponent::HandleControllerChanged()
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

void UBECharacterBasicComponent::HandlePlayerStateReplicated()
{
	CheckDefaultInitialization();
}

void UBECharacterBasicComponent::HandlePlayerInputComponentSetup()
{
	CheckDefaultInitialization();
}


void UBECharacterBasicComponent::InitializeAbilitySystem(UBEAbilitySystemComponent* InASC, AActor* InOwnerActor)
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
	AActor* ExistingAvatar = InASC->GetAvatarActor();

	UE_LOG(LogBE, Verbose, TEXT("Setting up ASC [%s] on pawn [%s] owner [%s], existing [%s] "), *GetNameSafe(InASC), *GetNameSafe(Pawn), *GetNameSafe(InOwnerActor), *GetNameSafe(ExistingAvatar));

	if ((ExistingAvatar != nullptr) && (ExistingAvatar != Pawn))
	{
		UE_LOG(LogBE, Log, TEXT("Existing avatar (authority=%d)"), ExistingAvatar->HasAuthority() ? 1 : 0);

		// There is already a pawn acting as the ASC's avatar, so we need to kick it out
		// This can happen on clients if they're lagged: their new pawn is spawned + possessed before the dead one is removed
		ensure(!ExistingAvatar->HasAuthority());

		if (UBECharacterBasicComponent* OtherExtensionComponent = FindCharacterBasicComponent(ExistingAvatar))
		{
			OtherExtensionComponent->UninitializeAbilitySystem();
		}
	}

	AbilitySystemComponent = InASC;
	AbilitySystemComponent->InitAbilityActorInfo(InOwnerActor, Pawn);

	if (ensure(CharacterData))
	{
		InASC->SetTagRelationshipMapping(CharacterData->TagRelationshipMapping);
	}

	OnAbilitySystemInitialized.Broadcast();

	CheckDefaultInitialization();
}

void UBECharacterBasicComponent::UninitializeAbilitySystem()
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

void UBECharacterBasicComponent::OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate Delegate)
{
	if (!OnAbilitySystemInitialized.IsBoundToObject(Delegate.GetUObject()))
	{
		OnAbilitySystemInitialized.Add(Delegate);
	}

	if (AbilitySystemComponent)
	{
		Delegate.Execute();
	}
}

void UBECharacterBasicComponent::OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate Delegate)
{
	if (!OnAbilitySystemUninitialized.IsBoundToObject(Delegate.GetUObject()))
	{
		OnAbilitySystemUninitialized.Add(Delegate);
	}
}


UBECharacterBasicComponent* UBECharacterBasicComponent::FindCharacterBasicComponent(const APawn* Pawn)
{
	return (Pawn ? Pawn->FindComponentByClass<UBECharacterBasicComponent>() : nullptr);
}

// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BEHealthComponent.h"

#include "System/BEAssetManager.h"
#include "System/BEGameData.h"
#include "Messages/BEVerbMessage.h"
#include "Messages/BEVerbMessageHelpers.h"
#include "Ability/BEAbilitySystemComponent.h"
#include "Ability/Attributes/BEHealthSet.h"
#include "BEGameplayTags.h"
#include "BELogChannels.h"

#include "Net/UnrealNetwork.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "GameplayPrediction.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEHealthComponent)

/////////////////////////////////////////

static AActor* GetInstigatorFromAttrChangeData(const FOnAttributeChangeData& ChangeData)
{
	if (ChangeData.GEModData != nullptr)
	{
		const FGameplayEffectContextHandle& EffectContext = ChangeData.GEModData->EffectSpec.GetEffectContext();
		return EffectContext.GetOriginalInstigator();
	}

	return nullptr;
}

/////////////////////////////////////////

UBEHealthComponent::UBEHealthComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);

	AbilitySystemComponent = nullptr;
	HealthSet = nullptr;
	DeathState = EBEDeathState::NotDead;

	DefaultMaxHealth = 100;
	DefaultMaxShield = 50;
	DefaultHealth	 = 100;
	DefaultShield = 50;
}

void UBEHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UBEHealthComponent, DeathState);
}

void UBEHealthComponent::InitializeWithAbilitySystem(UBEAbilitySystemComponent* InASC)
{
	AActor* Owner = GetOwner();
	check(Owner);

	if (AbilitySystemComponent)
	{
		UE_LOG(LogBE, Error, TEXT("BEHealthComponent: Health component for owner [%s] has already been initialized with an ability system."), *GetNameSafe(Owner));
		return;
	}

	AbilitySystemComponent = InASC;
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogBE, Error, TEXT("BEHealthComponent: Cannot initialize health component for owner [%s] with NULL ability system."), *GetNameSafe(Owner));
		return;
	}

	HealthSet = AbilitySystemComponent->GetSet<UBEHealthSet>();
	if (!HealthSet)
	{
		UE_LOG(LogBE, Error, TEXT("BEHealthComponent: Cannot initialize health component for owner [%s] with NULL health set on the ability system."), *GetNameSafe(Owner));
		return;
	}

	///////////////////////////////////
	// イベントバインド
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBEHealthSet::GetHealthAttribute()).AddUObject(this, &ThisClass::HandleHealthChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBEHealthSet::GetShieldAttribute()).AddUObject(this, &ThisClass::HandleShieldChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBEHealthSet::GetMaxHealthAttribute()).AddUObject(this, &ThisClass::HandleMaxHealthChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBEHealthSet::GetMaxShieldAttribute()).AddUObject(this, &ThisClass::HandleMaxShieldChanged);
	HealthSet->OnOutOfHealth.AddUObject(this, &ThisClass::HandleOutOfHealth);

	///////////////////////////////////
	// アトリビュートの値を初期化
	AbilitySystemComponent->SetNumericAttributeBase(UBEHealthSet::GetMaxHealthAttribute(), DefaultMaxHealth);
	AbilitySystemComponent->SetNumericAttributeBase(UBEHealthSet::GetMaxShieldAttribute(), DefaultMaxShield);
	AbilitySystemComponent->SetNumericAttributeBase(UBEHealthSet::GetHealthAttribute(), DefaultHealth);
	AbilitySystemComponent->SetNumericAttributeBase(UBEHealthSet::GetShieldAttribute(), DefaultShield);

	ClearGameplayTags();

	///////////////////////////////////
	// 初期値の更新を知らせる
	OnHealthChanged.Broadcast(this, HealthSet->GetHealth(), HealthSet->GetHealth(), nullptr);
	OnShieldChanged.Broadcast(this, HealthSet->GetShield(), HealthSet->GetShield(), nullptr);
	OnMaxHealthChanged.Broadcast(this, HealthSet->GetMaxHealth(), HealthSet->GetMaxHealth(), nullptr);
	OnMaxShieldChanged.Broadcast(this, HealthSet->GetMaxShield(), HealthSet->GetMaxShield(), nullptr);
}

void UBEHealthComponent::UninitializeFromAbilitySystem()
{
	ClearGameplayTags();

	if (HealthSet)
	{
		HealthSet->OnOutOfHealth.RemoveAll(this);
	}

	HealthSet = nullptr;
	AbilitySystemComponent = nullptr;
}

void UBEHealthComponent::OnUnregister()
{
	UninitializeFromAbilitySystem();

	Super::OnUnregister();
}


float UBEHealthComponent::GetHealth() const
{
	return (HealthSet ? HealthSet->GetHealth() : 0.0f);
}

float UBEHealthComponent::GetShield() const
{
	return (HealthSet ? HealthSet->GetShield() : 0.0f);
}

float UBEHealthComponent::GetMaxHealth() const
{
	return (HealthSet ? HealthSet->GetMaxHealth() : 1.0f);
}

float UBEHealthComponent::GetMaxShield() const
{
	return (HealthSet ? HealthSet->GetMaxShield() : 0.0f);
}

float UBEHealthComponent::GetTotalHealth() const
{
	return (GetHealth() + GetShield());
}

float UBEHealthComponent::GetTotalMaxHealth() const
{
	return (GetMaxHealth() + GetMaxShield());
}


void UBEHealthComponent::StartDeath()
{
	if (DeathState != EBEDeathState::NotDead)
	{
		return;
	}

	DeathState = EBEDeathState::DeathStarted;

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(TAG_Status_Death_Dying, 1);
	}

	AActor* Owner = GetOwner();
	check(Owner);

	OnDeathStarted.Broadcast(Owner);

	Owner->ForceNetUpdate();
}

void UBEHealthComponent::FinishDeath()
{
	if (DeathState != EBEDeathState::DeathStarted)
	{
		return;
	}

	DeathState = EBEDeathState::DeathFinished;

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(TAG_Status_Death_Dead, 1);
	}

	AActor* Owner = GetOwner();
	check(Owner);

	OnDeathFinished.Broadcast(Owner);

	Owner->ForceNetUpdate();
}

void UBEHealthComponent::HandleHealthChanged(const FOnAttributeChangeData& ChangeData)
{
	OnHealthChanged.Broadcast(this, ChangeData.OldValue, ChangeData.NewValue, GetInstigatorFromAttrChangeData(ChangeData));
}

void UBEHealthComponent::HandleShieldChanged(const FOnAttributeChangeData& ChangeData)
{
	OnShieldChanged.Broadcast(this, ChangeData.OldValue, ChangeData.NewValue, GetInstigatorFromAttrChangeData(ChangeData));
}

void UBEHealthComponent::HandleMaxHealthChanged(const FOnAttributeChangeData& ChangeData)
{
	OnMaxHealthChanged.Broadcast(this, ChangeData.OldValue, ChangeData.NewValue, GetInstigatorFromAttrChangeData(ChangeData));
}

void UBEHealthComponent::HandleMaxShieldChanged(const FOnAttributeChangeData& ChangeData)
{
	OnMaxShieldChanged.Broadcast(this, ChangeData.OldValue, ChangeData.NewValue, GetInstigatorFromAttrChangeData(ChangeData));
}

void UBEHealthComponent::HandleOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec& DamageEffectSpec, float DamageMagnitude)
{
#if WITH_SERVER_CODE
	if (AbilitySystemComponent)
	{
		// Send the "GameplayEvent.Death" gameplay event through the owner's ability system.  This can be used to trigger a death gameplay ability.
		{
			FGameplayEventData Payload;
			Payload.EventTag = TAG_Event_Death;
			Payload.Instigator = DamageInstigator;
			Payload.Target = AbilitySystemComponent->GetAvatarActor();
			Payload.OptionalObject = DamageEffectSpec.Def;
			Payload.ContextHandle = DamageEffectSpec.GetEffectContext();
			Payload.InstigatorTags = *DamageEffectSpec.CapturedSourceTags.GetAggregatedTags();
			Payload.TargetTags = *DamageEffectSpec.CapturedTargetTags.GetAggregatedTags();
			Payload.EventMagnitude = DamageMagnitude;

			FScopedPredictionWindow NewScopedWindow(AbilitySystemComponent, true);
			AbilitySystemComponent->HandleGameplayEvent(Payload.EventTag, &Payload);
		}

		// Send a standardized verb message that other systems can observe
		{
			FBEVerbMessage Message;
			Message.Verb = TAG_Message_Elimination;
			Message.Instigator = DamageInstigator;
			Message.InstigatorTags = *DamageEffectSpec.CapturedSourceTags.GetAggregatedTags();
			Message.Target = UBEVerbMessageHelpers::GetPlayerStateFromObject(AbilitySystemComponent->GetAvatarActor());
			Message.TargetTags = *DamageEffectSpec.CapturedTargetTags.GetAggregatedTags();
			//@TODO: Fill out context tags, and any non-ability-system source/instigator tags
			//@TODO: Determine if it's an opposing team kill, self-own, team kill, etc...

			UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
			MessageSystem.BroadcastMessage(Message.Verb, Message);
		}

		//@TODO: assist messages (could compute from damage dealt elsewhere)?
	}

#endif // #if WITH_SERVER_CODE
}

void UBEHealthComponent::OnRep_DeathState(EBEDeathState OldDeathState)
{
	const EBEDeathState NewDeathState = DeathState;

	// Revert the death state for now since we rely on StartDeath and FinishDeath to change it.
	DeathState = OldDeathState;

	if (OldDeathState > NewDeathState)
	{
		// The server is trying to set us back but we've already predicted past the server state.
		UE_LOG(LogBE, Warning, TEXT("BEHealthComponent: Predicted past server death state [%d] -> [%d] for owner [%s]."), (uint8)OldDeathState, (uint8)NewDeathState, *GetNameSafe(GetOwner()));
		return;
	}

	if (OldDeathState == EBEDeathState::NotDead)
	{
		if (NewDeathState == EBEDeathState::DeathStarted)
		{
			StartDeath();
		}
		else if (NewDeathState == EBEDeathState::DeathFinished)
		{
			StartDeath();
			FinishDeath();
		}
		else
		{
			UE_LOG(LogBE, Error, TEXT("BEHealthComponent: Invalid death transition [%d] -> [%d] for owner [%s]."), (uint8)OldDeathState, (uint8)NewDeathState, *GetNameSafe(GetOwner()));
		}
	}
	else if (OldDeathState == EBEDeathState::DeathStarted)
	{
		if (NewDeathState == EBEDeathState::DeathFinished)
		{
			FinishDeath();
		}
		else
		{
			UE_LOG(LogBE, Error, TEXT("BEHealthComponent: Invalid death transition [%d] -> [%d] for owner [%s]."), (uint8)OldDeathState, (uint8)NewDeathState, *GetNameSafe(GetOwner()));
		}
	}

	ensureMsgf((DeathState == NewDeathState), TEXT("BEHealthComponent: Death transition failed [%d] -> [%d] for owner [%s]."), (uint8)OldDeathState, (uint8)NewDeathState, *GetNameSafe(GetOwner()));
}


void UBEHealthComponent::DamageSelfDestruct(bool bFellOutOfWorld)
{
	if ((DeathState == EBEDeathState::NotDead) && AbilitySystemComponent)
	{
		const TSubclassOf<UGameplayEffect> DamageGE = UBEAssetManager::GetSubclass(UBEGameData::Get().DamageGameplayEffect_SetByCaller);
		if (!DamageGE)
		{
			UE_LOG(LogBE, Error, TEXT("BEHealthComponent: DamageSelfDestruct failed for owner [%s]. Unable to find gameplay effect [%s]."), *GetNameSafe(GetOwner()), *UBEGameData::Get().DamageGameplayEffect_SetByCaller.GetAssetName());
			return;
		}

		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DamageGE, 1.0f, AbilitySystemComponent->MakeEffectContext());
		FGameplayEffectSpec* Spec = SpecHandle.Data.Get();

		if (!Spec)
		{
			UE_LOG(LogBE, Error, TEXT("BEHealthComponent: DamageSelfDestruct failed for owner [%s]. Unable to make outgoing spec for [%s]."), *GetNameSafe(GetOwner()), *GetNameSafe(DamageGE));
			return;
		}

		Spec->AddDynamicAssetTag(TAG_Gameplay_Damage_SelfDestruct);

		if (bFellOutOfWorld)
		{
			Spec->AddDynamicAssetTag(TAG_Gameplay_Damage_FellOutOfWorld);
		}

		const float DamageAmount = GetTotalMaxHealth();

		Spec->SetSetByCallerMagnitude(TAG_SetByCaller_Damage, DamageAmount);
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec);
	}
}

void UBEHealthComponent::ClearGameplayTags()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(TAG_Status_Death_Dying, 0);
		AbilitySystemComponent->SetLooseGameplayTagCount(TAG_Status_Death_Dead, 0);
	}
}
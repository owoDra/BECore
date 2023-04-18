// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BEHealthSet.h"

#include "Ability/BEAbilitySystemComponent.h"
#include "Messages/BEVerbMessage.h"
#include "BEGameplayTags.h"

#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/GameplayMessageSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEHealthSet)

///////////////////////////////////////////////////

UBEHealthSet::UBEHealthSet()
	: Health(100.0f)
	, Shield(50.0f)
	, MaxHealth(100.0f)
	, MaxShield(50.0f)
	, DamageResistance(1.0f)
{
	bOutOfHealth = false;
}

void UBEHealthSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UBEHealthSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBEHealthSet, Shield, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBEHealthSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBEHealthSet, MaxShield, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBEHealthSet, DamageResistance, COND_None, REPNOTIFY_Always);
}

void UBEHealthSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBEHealthSet, Health, OldValue);
}

void UBEHealthSet::OnRep_Shield(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBEHealthSet, Shield, OldValue);
}

void UBEHealthSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBEHealthSet, MaxHealth, OldValue);
}

void UBEHealthSet::OnRep_MaxShield(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBEHealthSet, MaxShield, OldValue);
}

void UBEHealthSet::OnRep_DamageResistance(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBEHealthSet, DamageResistance, OldValue);
}

///////////////////////////////////////////////////

bool UBEHealthSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData &Data)
{
	if (!Super::PreGameplayEffectExecute(Data))
	{
		return false;
	}

	// Damage が変更されたとき
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		if (Data.EvaluatedData.Magnitude > 0.0f)
		{
			const bool bIsDamageFromSelfDestruct = Data.EffectSpec.GetDynamicAssetTags().HasTagExact(TAG_Gameplay_Damage_SelfDestruct);

			if (Data.Target.HasMatchingGameplayTag(TAG_Status_DamageImmunity) && !bIsDamageFromSelfDestruct)
			{
				// Do not take away any health.
				Data.EvaluatedData.Magnitude = 0.0f;
				return false;
			}

#if !UE_BUILD_SHIPPING
			// Check GodMode cheat, unlimited health is checked below
			if (Data.Target.HasMatchingGameplayTag(TAG_Cheat_UnlimitedHealth) && !bIsDamageFromSelfDestruct)
			{
				// Do not take away any health.
				Data.EvaluatedData.Magnitude = 0.0f;
				return false;
			}
#endif // #if !UE_BUILD_SHIPPING

			Data.EvaluatedData.Magnitude *= GetDamageResistance();
		}
	}

	return true;
}

void UBEHealthSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// Damage の値を Health と Shield に適用する
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		if (Data.EvaluatedData.Magnitude > 0.0f)
		{
			FBEVerbMessage Message;
			Message.Verb = TAG_Message_Damage;
			Message.Instigator = Data.EffectSpec.GetEffectContext().GetEffectCauser();
			Message.InstigatorTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
			Message.Target = GetOwningActor();
			Message.TargetTags = *Data.EffectSpec.CapturedTargetTags.GetAggregatedTags();
			//@TODO: Fill out context tags, and any non-ability-system source/instigator tags
			//@TODO: Determine if it's an opposing team kill, self-own, team kill, etc...
			Message.Magnitude = Data.EvaluatedData.Magnitude;

			UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
			MessageSystem.BroadcastMessage(Message.Verb, Message);

			float DamageToShield = GetDamage();
			float DamageToHealth = GetDamage() - GetShield();

			SetShield(FMath::Clamp(GetShield() - DamageToShield, 0.0f, GetMaxShield()));
			if (DamageToHealth > 0.0)
			{
				SetHealth(FMath::Clamp(GetHealth() - DamageToHealth, 0.0f, GetMaxHealth()));
			}
		}
		SetDamage(0.0f);
	}

	// HealingHealth の値を Health に適用する
	else if (Data.EvaluatedData.Attribute == GetHealingHealthAttribute())
	{
		if (GetHealingHealth() > 0 && GetHealth() < GetMaxHealth())
		{
			SetHealth(FMath::Clamp(GetHealth() + GetHealingHealth(), 0.0f, GetMaxHealth()));
		}
		SetHealingHealth(0.0f);
	}

	// HealingShield の値を Shield に適用する
	else if (Data.EvaluatedData.Attribute == GetHealingShieldAttribute())
	{
		if (GetHealingShield() > 0 && GetShield() < GetMaxShield())
		{
			SetShield(FMath::Clamp(GetShield() + GetHealingShield(), 0.0f, GetMaxShield()));
		}
		SetHealingShield(0.0f);
	}

	// GameplayEffect による Health の変更を 0～MaxHealth の範囲で制限
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	}

	// GameplayEffect による Shield の変更を 0～MaxShield の範囲で制限
	else if (Data.EvaluatedData.Attribute == GetShieldAttribute())
	{
		SetShield(FMath::Clamp(GetShield(), 0.0f, GetMaxShield()));
	}

	// Health が 0.0 以下ならば Death を知らせる
	if ((GetHealth() <= 0.0f) && !bOutOfHealth)
	{
		if (OnOutOfHealth.IsBound())
		{
			const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
			AActor* Instigator = EffectContext.GetOriginalInstigator();
			AActor* Causer = EffectContext.GetEffectCauser();

			OnOutOfHealth.Broadcast(Instigator, Causer, Data.EffectSpec, Data.EvaluatedData.Magnitude);
		}
	}
	bOutOfHealth = (GetHealth() <= 0.0f);
}

void UBEHealthSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

void UBEHealthSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

void UBEHealthSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	// Health の最大値を MaxHealth で制限。
	if (Attribute == GetMaxHealthAttribute())
	{
		if (GetHealth() > GetMaxHealth())
		{
			UBEAbilitySystemComponent* BEASC = GetBEAbilitySystemComponent();
			check(BEASC);

			BEASC->ApplyModToAttribute(GetHealthAttribute(), EGameplayModOp::Override, GetMaxHealth());
		}
	}

	// Shield の最大値を MaxShield で制限。
	if (Attribute == GetMaxShieldAttribute())
	{
		if (GetShield() > GetMaxShield())
		{
			UBEAbilitySystemComponent* BEASC = GetBEAbilitySystemComponent();
			check(BEASC);

			BEASC->ApplyModToAttribute(GetShieldAttribute(), EGameplayModOp::Override, GetMaxShield());
		}
	}

	if (bOutOfHealth && (GetHealth() > 0.0f))
	{
		bOutOfHealth = false;
	}
}

void UBEHealthSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetHealthAttribute())
	{
		// Health の最低値は 0.0, 最大値は MaxHealth
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetShieldAttribute())
	{
		// Shield の最低値は 0.0, 最大値はMaxShield
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxShield());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		// MaxHealth の最低値は 1.0
		NewValue = FMath::Max(NewValue, 1.0f);
	}
	else if (Attribute == GetMaxShieldAttribute())
	{
		// MaxShield の最低値は 0.0
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetDamageResistanceAttribute())
	{
		// DamageResistance の最低値は 0.0
		NewValue = FMath::Max(NewValue, 0.0f);
	}
}

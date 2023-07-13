// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#pragma once

#include "BEAttributeSet.h"

#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "Misc/AssertionMacros.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"

#include "BECombatSet.generated.h"

class UObject;
struct FFrame;

/////////////////////////////////////////////////

/**
 * UBECombatSet
 *
 *  戦闘におけるダメージや回復などの Attribute を定義する
 */
UCLASS(BlueprintType)
class UBECombatSet : public UBEAttributeSet
{
	GENERATED_BODY()

public:

	UBECombatSet();

	ATTRIBUTE_ACCESSORS(UBECombatSet, BaseDamage);
	ATTRIBUTE_ACCESSORS(UBECombatSet, BaseHealHealth);
	ATTRIBUTE_ACCESSORS(UBECombatSet, BaseHealShield);

protected:

	UFUNCTION()
	void OnRep_BaseDamage(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_BaseHealHealth(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_BaseHealShield(const FGameplayAttributeData& OldValue);

private:

	// DamageExecution で適応するためのベースダメージ
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BaseDamage, Category = "BE|Combat", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData BaseDamage;

	// HealHealthExecution で適応するためのベース回復量
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BaseHealHealth, Category = "BE|Combat", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData BaseHealHealth;

	// HealShieldExecution で適応するためのベース回復量
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BaseHealShield, Category = "BE|Combat", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData BaseHealShield;
};

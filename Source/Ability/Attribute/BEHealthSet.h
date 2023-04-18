// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "BEAttributeSet.h"

#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "Misc/AssertionMacros.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"

#include "BEHealthSet.generated.h"

class UObject;
struct FFrame;
struct FGameplayEffectModCallbackData;

///////////////////////////////////////////////////

/**
 * UBEHealthSet
 *
 *  キャラクターの体力や体力上限、ダメージ耐性などの Attribute を定義する
 */
UCLASS(BlueprintType)
class UBEHealthSet : public UBEAttributeSet
{
	GENERATED_BODY()

public:
	UBEHealthSet();

	ATTRIBUTE_ACCESSORS(UBEHealthSet, Health);
	ATTRIBUTE_ACCESSORS(UBEHealthSet, Shield);
	ATTRIBUTE_ACCESSORS(UBEHealthSet, MaxHealth);
	ATTRIBUTE_ACCESSORS(UBEHealthSet, MaxShield);

	ATTRIBUTE_ACCESSORS(UBEHealthSet, HealingHealth);
	ATTRIBUTE_ACCESSORS(UBEHealthSet, HealingShield);
	ATTRIBUTE_ACCESSORS(UBEHealthSet, Damage);

	ATTRIBUTE_ACCESSORS(UBEHealthSet, DamageResistance);

	// Delegate to broadcast when the health attribute reaches zero.
	mutable FBEAttributeEvent OnOutOfHealth;

protected:

	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Shield(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxShield(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_DamageResistance(const FGameplayAttributeData& OldValue);

	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;

private:
	/**
	 * 現在のHP。
	 * 最大値は MaxHealth によって決まる。
	 * このアトリビュートは Modifier に表示されず Execution によってのみ変更される。
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "BE|Health", Meta = (HideFromModifiers, AllowPrivateAccess = true))
	FGameplayAttributeData Health;

	/**
	 * 現在のシールド値。
	 * 最大値は MaxShield によって決まる。
	 * このアトリビュートは Modifier に表示されず Execution によってのみ変更される。
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Shield, Category = "BE|Health", Meta = (HideFromModifiers, AllowPrivateAccess = true))
	FGameplayAttributeData Shield;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "BE|Health", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxHealth;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxShield, Category = "BE|Health", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxShield;

	// 現在のダメージ耐性(被ダメージ倍率)
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DamageResistance, Category = "BE|Health", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData DamageResistance;

	// Used to track when the health reaches 0.
	bool bOutOfHealth;

	//============================================
	//	メタ属性 (以下は「ステートフル」ではない属性)
	//============================================
private:
	// HP回復量。この値をもとに直接 Health の値を増加させる。
	UPROPERTY(BlueprintReadOnly, Category="BE|Health", Meta=(AllowPrivateAccess=true))
	FGameplayAttributeData HealingHealth;

	// シールド回復量。この値をもとに直接 Shield の値を増加させる。
	UPROPERTY(BlueprintReadOnly, Category = "BE|Health", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData HealingShield;

	// ダメージ量。この値をもとに直接 Health の値を減少させる。
	UPROPERTY(BlueprintReadOnly, Category="BE|Health", Meta=(HideFromModifiers, AllowPrivateAccess=true))
	FGameplayAttributeData Damage;
};

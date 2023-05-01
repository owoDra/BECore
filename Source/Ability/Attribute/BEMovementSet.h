// Copyright Eigi Chin

#pragma once

#include "BEAttributeSet.h"

#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "Misc/AssertionMacros.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"

#include "BEMovementSet.generated.h"

class UObject;
struct FFrame;
struct FGameplayEffectModCallbackData;


/**
 * UBEMovementSet
 *
 *	キャラクターの移動系のアトリビュートを定義する
 */
UCLASS(BlueprintType)
class UBEMovementSet : public UBEAttributeSet
{
	GENERATED_BODY()
public:
	UBEMovementSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


	//======================================
	//	アトリビュートの宣言
	//======================================
public:
	ATTRIBUTE_ACCESSORS(UBEMovementSet, GravityScale);
	ATTRIBUTE_ACCESSORS(UBEMovementSet, GroundFriction);

	ATTRIBUTE_ACCESSORS(UBEMovementSet, OverallSpeedMultiplier);
	ATTRIBUTE_ACCESSORS(UBEMovementSet, WalkSpeed);
	ATTRIBUTE_ACCESSORS(UBEMovementSet, WalkSpeedCrouched);
	ATTRIBUTE_ACCESSORS(UBEMovementSet, WalkSpeedRunning);
	ATTRIBUTE_ACCESSORS(UBEMovementSet, WalkSpeedTargeting);
	ATTRIBUTE_ACCESSORS(UBEMovementSet, SwimSpeed);
	ATTRIBUTE_ACCESSORS(UBEMovementSet, FlySpeed);

	ATTRIBUTE_ACCESSORS(UBEMovementSet, JumpPower);
	ATTRIBUTE_ACCESSORS(UBEMovementSet, AirControl);


	//======================================
	//	アトリビュートのレプリケート
	//======================================
protected:

	UFUNCTION()
		void OnRep_GravityScale(const FGameplayAttributeData& OldValue);

	UFUNCTION()
		void OnRep_GroundFriction(const FGameplayAttributeData& OldValue);


	UFUNCTION()
		void OnRep_OverallSpeedMultiplier(const FGameplayAttributeData& OldValue);

	UFUNCTION()
		void OnRep_WalkSpeed(const FGameplayAttributeData& OldValue);

	UFUNCTION()
		void OnRep_WalkSpeedCrouched(const FGameplayAttributeData& OldValue);

	UFUNCTION()
		void OnRep_WalkSpeedRunning(const FGameplayAttributeData& OldValue);

	UFUNCTION()
		void OnRep_WalkSpeedTargeting(const FGameplayAttributeData& OldValue);

	UFUNCTION()
		void OnRep_SwimSpeed(const FGameplayAttributeData& OldValue);

	UFUNCTION()
		void OnRep_FlySpeed(const FGameplayAttributeData& OldValue);


	UFUNCTION()
		void OnRep_JumpPower(const FGameplayAttributeData& OldValue);

	UFUNCTION()
		void OnRep_AirControl(const FGameplayAttributeData& OldValue);


	//======================================
	//	アトリビュートデータ
	//======================================
private:
	// 重力の影響度
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_GravityScale, Category = "BE|Movement", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData GravityScale;

	// 地面の摩擦
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_GroundFriction, Category = "BE|Movement", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData GroundFriction;


	// 全体の移動速度倍率 
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_OverallSpeedMultiplier, Category = "BE|Movement", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData OverallSpeedMultiplier;

	// 歩く速度 
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_WalkSpeed, Category = "BE|Movement", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData WalkSpeed;

	// しゃがみ歩きの速度
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_WalkSpeedCrouched, Category = "BE|Movement", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData WalkSpeedCrouched;

	// 走りの速度
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_WalkSpeedRunning, Category = "BE|Movement", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData WalkSpeedRunning;

	// エイム時の移動速度
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_WalkSpeedTargeting, Category = "BE|Movement", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData WalkSpeedTargeting;

	// 泳ぐ速度 
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_SwimSpeed, Category = "BE|Movement", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData SwimSpeed;

	// 飛ぶ速度
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_FlySpeed, Category = "BE|Movement", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData FlySpeed;


	// ジャンプ力 
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_JumpPower, Category = "BE|Movement", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData JumpPower;

	// 空中操作の自由度
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AirControl, Category = "BE|Movement", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData AirControl;
};

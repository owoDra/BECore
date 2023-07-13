// Copyright owoDra

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


public:
	ATTRIBUTE_ACCESSORS(UBEMovementSet, GravityScale);
	ATTRIBUTE_ACCESSORS(UBEMovementSet, GroundFrictionScale);
	ATTRIBUTE_ACCESSORS(UBEMovementSet, MoveSpeedScale);
	ATTRIBUTE_ACCESSORS(UBEMovementSet, JumpPowerScale);
	ATTRIBUTE_ACCESSORS(UBEMovementSet, AirControlScale);

protected:
	UFUNCTION() void OnRep_GravityScale(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_GroundFrictionScale(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_MoveSpeedScale(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_JumpPowerScale(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_AirControlScale(const FGameplayAttributeData& OldValue);

private:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_GravityScale, Category = "BE|Movement", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData GravityScale;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_GroundFrictionScale, Category = "BE|Movement", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData GroundFrictionScale;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MoveSpeedScale, Category = "BE|Movement", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MoveSpeedScale;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_JumpPowerScale, Category = "BE|Movement", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData JumpPowerScale;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AirControlScale, Category = "BE|Movement", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData AirControlScale;
};

// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "Ability/BEGameplayAbility.h"

#include "GameplayAbilitySpec.h"
#include "UObject/UObjectGlobals.h"

#include "BEGameplayAbility_Reset.generated.h"

class AActor;
class UObject;
struct FGameplayAbilityActorInfo;
struct FGameplayEventData;


/**
 * UBEGameplayAbility_Reset
 *
 *	キャラクターをリセットするアビリティ
 *	Gameplay Event "Event.Reset"によって自動的に実行される(サーバーのみ)
 */
UCLASS()
class BECORE_API UBEGameplayAbility_Reset : public UBEGameplayAbility
{
	GENERATED_BODY()

public:
	UBEGameplayAbility_Reset(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	
	void DoneAddingNativeTags();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};


USTRUCT(BlueprintType)
struct FBEPlayerResetMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	AActor* OwnerPlayerState = nullptr;
};
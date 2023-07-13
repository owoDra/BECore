// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#pragma once

#include "Engine/DataAsset.h"

#include "UObject/SoftObjectPtr.h"
#include "UObject/UObjectGlobals.h"

#include "BEGameData.generated.h"

class UGameplayEffect;
class UObject;

/////////////////////////////////////////////////

/**
 * UBEGameData
 *
 *	Non-mutable data asset that contains global game data.
 */
UCLASS(BlueprintType, Const, Meta = (DisplayName = "BE Game Data", ShortTooltip = "Data asset containing global game data."))
class UBEGameData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UBEGameData();

	// Returns the loaded game data.
	static const UBEGameData& Get();

public:

	// ダメージを与えるために使用される GameplayEffect。ダメージ量には Set By Caller を使用。
	UPROPERTY(EditDefaultsOnly, Category = "Default Gameplay Effects", meta = (DisplayName = "Damage Gameplay Effect (SetByCaller)"))
	TSoftClassPtr<UGameplayEffect> DamageGameplayEffect_SetByCaller;

	// HP回復を与えるために使用される GameplayEffect。回復量には Set By Caller を使用。
	UPROPERTY(EditDefaultsOnly, Category = "Default Gameplay Effects", meta = (DisplayName = "Heal Health Gameplay Effect (SetByCaller)"))
	TSoftClassPtr<UGameplayEffect> HealHealthGameplayEffect_SetByCaller;

	// シールド回復を与えるために使用される GameplayEffect。回復量には Set By Caller を使用。
	UPROPERTY(EditDefaultsOnly, Category = "Default Gameplay Effects", meta = (DisplayName = "Heal Shield Gameplay Effect (SetByCaller)"))
	TSoftClassPtr<UGameplayEffect> HealShieldGameplayEffect_SetByCaller;

	// Gameplay effect used to add and remove dynamic tags.
	UPROPERTY(EditDefaultsOnly, Category = "Default Gameplay Effects")
	TSoftClassPtr<UGameplayEffect> DynamicTagGameplayEffect;
};

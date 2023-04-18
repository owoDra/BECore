// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "AbilitySystemGlobals.h"

#include "UObject/UObjectGlobals.h"

#include "BEAbilitySystemGlobals.generated.h"

class UObject;
struct FGameplayEffectContext;


UCLASS(Config=Game)
class UBEAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_UCLASS_BODY()

	//~UAbilitySystemGlobals interface
	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
	//~End of UAbilitySystemGlobals interface
};

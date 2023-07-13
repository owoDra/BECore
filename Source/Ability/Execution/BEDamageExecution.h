// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#pragma once

#include "GameplayEffectExecutionCalculation.h"

#include "UObject/UObjectGlobals.h"

#include "BEDamageExecution.generated.h"

class UObject;


/**
 * UBEDamageExecution
 *
 *	ダメージを与えるための Gameplay Effect の Exection
 */
UCLASS()
class UBEDamageExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:

	UBEDamageExecution();

protected:

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};

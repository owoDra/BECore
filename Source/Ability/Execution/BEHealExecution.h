// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "GameplayEffectExecutionCalculation.h"

#include "UObject/UObjectGlobals.h"

#include "BEHealExecution.generated.h"

class UObject;

/////////////////////////////////////////////////

/**
 * UBEHealHealthExecution
 *
 *	HP回復のための Gameplay Effect の Exection
 */
UCLASS()
class UBEHealHealthExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:

	UBEHealHealthExecution();

protected:

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};

/////////////////////////////////////////////////

/**
 * UBEHealShieldExecution
 *
 *	シールド回復のための Gameplay Effect の Exection
 */
UCLASS()
class UBEHealShieldExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:

	UBEHealShieldExecution();

protected:

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};

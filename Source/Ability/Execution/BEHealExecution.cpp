// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BEHealExecution.h"

#include "Ability/Attribute/BEHealthSet.h"
#include "Ability/Attribute/BECombatSet.h"

#include "GameplayEffectTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEHealExecution)

/////////////////////////////////////////////////

struct FHealHealthStatics
{
	FGameplayEffectAttributeCaptureDefinition BaseHealDef;

	FHealHealthStatics()
	{
		BaseHealDef = FGameplayEffectAttributeCaptureDefinition(UBECombatSet::GetBaseHealHealthAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);
	}
};

static FHealHealthStatics& HealHealthStatics()
{
	static FHealHealthStatics Statics;
	return Statics;
}

struct FHealShieldStatics
{
	FGameplayEffectAttributeCaptureDefinition BaseHealDef;

	FHealShieldStatics()
	{
		BaseHealDef = FGameplayEffectAttributeCaptureDefinition(UBECombatSet::GetBaseHealShieldAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);
	}
};

static FHealShieldStatics& HealShieldStatics()
{
	static FHealShieldStatics Statics;
	return Statics;
}

/////////////////////////////////////////////////

UBEHealHealthExecution::UBEHealHealthExecution()
{
	RelevantAttributesToCapture.Add(HealHealthStatics().BaseHealDef);
}

void UBEHealHealthExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
#if WITH_SERVER_CODE
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	float BaseHeal = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(HealHealthStatics().BaseHealDef, EvaluateParameters, BaseHeal);

	const float HealingDone = FMath::Max(0.0f, BaseHeal);

	if (HealingDone > 0.0f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UBEHealthSet::GetHealingHealthAttribute(), EGameplayModOp::Additive, HealingDone));
	}
#endif // #if WITH_SERVER_CODE
}

/////////////////////////////////////////////////

UBEHealShieldExecution::UBEHealShieldExecution()
{
	RelevantAttributesToCapture.Add(HealShieldStatics().BaseHealDef);
}

void UBEHealShieldExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
#if WITH_SERVER_CODE
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	float BaseHeal = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(HealShieldStatics().BaseHealDef, EvaluateParameters, BaseHeal);

	const float HealingDone = FMath::Max(0.0f, BaseHeal);

	if (HealingDone > 0.0f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UBEHealthSet::GetHealingShieldAttribute(), EGameplayModOp::Additive, HealingDone));
	}
#endif // #if WITH_SERVER_CODE
}

// Copyright Eigi Chin

#include "BEMovementSet.h"

#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEMovementSet)


UBEMovementSet::UBEMovementSet()
	: GravityScale(2.0f)
	, GroundFriction(8.0f)
	, OverallSpeedMultiplier(1.0f)
	, WalkSpeed(200.0f)
	, WalkSpeedCrouched(200.0f)
	, WalkSpeedRunning(400.0f)
	, WalkSpeedAiming(200.0f)
	, SwimSpeed(400.0f)
	, FlySpeed(400.0f)
	, JumpPower(400.0f)
	, AirControl(0.2f)
{
}

void UBEMovementSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UBEMovementSet, GravityScale, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBEMovementSet, GroundFriction, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UBEMovementSet, WalkSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBEMovementSet, OverallSpeedMultiplier, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBEMovementSet, WalkSpeedCrouched, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBEMovementSet, WalkSpeedRunning, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBEMovementSet, WalkSpeedAiming, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBEMovementSet, SwimSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBEMovementSet, FlySpeed, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UBEMovementSet, JumpPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBEMovementSet, AirControl, COND_None, REPNOTIFY_Always);
}


void UBEMovementSet::OnRep_GravityScale(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBEMovementSet, GravityScale, OldValue);
}

void UBEMovementSet::OnRep_GroundFriction(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBEMovementSet, GroundFriction, OldValue);
}


void UBEMovementSet::OnRep_OverallSpeedMultiplier(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBEMovementSet, OverallSpeedMultiplier, OldValue);
}

void UBEMovementSet::OnRep_WalkSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBEMovementSet, WalkSpeed, OldValue);
}

void UBEMovementSet::OnRep_WalkSpeedCrouched(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBEMovementSet, WalkSpeedCrouched, OldValue);
}

void UBEMovementSet::OnRep_WalkSpeedRunning(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBEMovementSet, WalkSpeedRunning, OldValue);
}

void UBEMovementSet::OnRep_WalkSpeedAiming(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBEMovementSet, WalkSpeedAiming, OldValue);
}

void UBEMovementSet::OnRep_SwimSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBEMovementSet, SwimSpeed, OldValue);
}

void UBEMovementSet::OnRep_FlySpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBEMovementSet, FlySpeed, OldValue);
}


void UBEMovementSet::OnRep_JumpPower(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBEMovementSet, JumpPower, OldValue);
}

void UBEMovementSet::OnRep_AirControl(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBEMovementSet, AirControl, OldValue);
}

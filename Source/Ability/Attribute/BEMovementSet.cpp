// Copyright owoDra

#include "BEMovementSet.h"

#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEMovementSet)


UBEMovementSet::UBEMovementSet()
	: GravityScale(1.0f)
	, GroundFrictionScale(1.0f)
	, MoveSpeedScale(1.0f)
	, JumpPowerScale(1.0f)
	, AirControlScale(1.0f)
{
}

void UBEMovementSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UBEMovementSet, GravityScale			, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBEMovementSet, GroundFrictionScale	, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBEMovementSet, MoveSpeedScale		, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBEMovementSet, JumpPowerScale		, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBEMovementSet, AirControlScale		, COND_None, REPNOTIFY_Always);
}


void UBEMovementSet::OnRep_GravityScale(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBEMovementSet, GravityScale, OldValue);
}

void UBEMovementSet::OnRep_GroundFrictionScale(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBEMovementSet, GroundFrictionScale, OldValue);
}

void UBEMovementSet::OnRep_MoveSpeedScale(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBEMovementSet, MoveSpeedScale, OldValue);
}

void UBEMovementSet::OnRep_JumpPowerScale(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBEMovementSet, JumpPowerScale, OldValue);
}

void UBEMovementSet::OnRep_AirControlScale(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBEMovementSet, AirControlScale, OldValue);
}

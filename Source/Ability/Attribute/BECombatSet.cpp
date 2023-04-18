// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BECombatSet.h"

#include "Containers/Array.h"
#include "Net/UnrealNetwork.h"
#include "UObject/CoreNetTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BECombatSet)

/////////////////////////////////////////////////

UBECombatSet::UBECombatSet()
	: BaseDamage(0.0f)
	, BaseHealHealth(0.0f)
	, BaseHealShield(0.0f)
{
}

void UBECombatSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UBECombatSet, BaseDamage, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBECombatSet, BaseHealHealth, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBECombatSet, BaseHealShield, COND_OwnerOnly, REPNOTIFY_Always);
}

void UBECombatSet::OnRep_BaseDamage(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBECombatSet, BaseDamage, OldValue);
}

void UBECombatSet::OnRep_BaseHealHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBECombatSet, BaseHealHealth, OldValue);
}

void UBECombatSet::OnRep_BaseHealShield(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBECombatSet, BaseHealShield, OldValue);
}

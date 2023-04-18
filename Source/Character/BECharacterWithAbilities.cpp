// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BECharacterWithAbilities.h"

#include "Ability/BEAbilitySystemComponent.h"
#include "Ability/Attributes/BEHealthSet.h"
#include "Ability/Attributes/BECombatSet.h"
#include "Ability/Attributes/BEMovementSet.h"

#include "AbilitySystemComponent.h"
#include "HAL/Platform.h"
#include "Misc/AssertionMacros.h"


ABECharacterWithAbilities::ABECharacterWithAbilities(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UBEAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	HealthSet = CreateDefaultSubobject<UBEHealthSet>(TEXT("HealthSet"));
	CombatSet = CreateDefaultSubobject<UBECombatSet>(TEXT("CombatSet"));
	MovementSet = CreateDefaultSubobject<UBEMovementSet>(TEXT("MovementSet"));

	// AbilitySystemComponent needs to be updated at a high frequency.
	NetUpdateFrequency = 100.0f;
}

void ABECharacterWithAbilities::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	check(AbilitySystemComponent);
	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	OnAbilitySystemInitialized();
}

UAbilitySystemComponent* ABECharacterWithAbilities::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

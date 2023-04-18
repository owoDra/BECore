// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "Character/BECharacter.h"

#include "UObject/UObjectGlobals.h"

#include "BECharacterWithAbilities.generated.h"

class UAbilitySystemComponent;
class UBEAbilitySystemComponent;
class UBEHealthSet;
class UBECombatSet;
class UBEMovementSet;
class UObject;


// ABECharacter typically gets the ability system component from the possessing player state
// This represents a character with a self-contained ability system component.
UCLASS(Blueprintable)
class BECORE_API ABECharacterWithAbilities : public ABECharacter
{
	GENERATED_BODY()

public:
	ABECharacterWithAbilities(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBEAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBEHealthSet> HealthSet;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBECombatSet> CombatSet;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBEMovementSet> MovementSet;
};

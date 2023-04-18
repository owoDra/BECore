// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "Engine/DataAsset.h"

#include "Containers/Array.h"
#include "Templates/SubclassOf.h"
#include "UObject/UObjectGlobals.h"

#include "BEPawnData.generated.h"

class APawn;
class UBEAbilitySet;
class UBEInputConfig;
class UBEAbilityTagRelationshipMapping;
class UBECameraMode;
class UObject;


/**
 * UBEPawnData
 *
 *	Pawn を定義するための Data Asset
 */
UCLASS(BlueprintType, Const, Meta = (DisplayName = "BE Pawn Data", ShortTooltip = "Data asset used to define a Pawn."))
class UBEPawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UBEPawnData(const FObjectInitializer& ObjectInitializer);

public:

	// Class to instantiate for this pawn (should usually derive from ABEPawn or ABECharacter).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pawn")
	TSubclassOf<APawn> PawnClass;

	// Ability sets to grant to this pawn's ability system.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TArray<TObjectPtr<UBEAbilitySet>> AbilitySets;

	// What mapping of ability tags to use for actions taking by this pawn
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UBEAbilityTagRelationshipMapping> TagRelationshipMapping;

	// Input configuration used by player controlled pawns to create input mappings and bind input actions.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UBEInputConfig> InputConfig;

	// Default camera mode used by player controlled pawns.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	TSubclassOf<UBECameraMode> DefaultCameraMode;
};

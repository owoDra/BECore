// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#pragma once

#include "PhysicalMaterials/PhysicalMaterial.h"

#include "GameplayTagContainer.h"
#include "UObject/UObjectGlobals.h"

#include "PhysicalMaterialWithTags.generated.h"

/**
 * UBEWeaponInstance
 *
 * A piece of equipment representing a weapon spawned and applied to a pawn
 */
UCLASS()
class UPhysicalMaterialWithTags : public UPhysicalMaterial
{
	GENERATED_BODY()

public:
	UPhysicalMaterialWithTags(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// A container of gameplay tags that game code can use to reason about this physical material
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=PhysicalProperties)
	FGameplayTagContainer Tags;
};

// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "Inventory/BEInventoryItemDefinition.h"

#include "Internationalization/Text.h"
#include "Math/Color.h"
#include "UObject/ObjectPtr.h"
#include "UObject/UObjectGlobals.h"

#include "InventoryFragment_PickupIcon.generated.h"

class UObject;
class USkeletalMesh;


UCLASS()
class UInventoryFragment_PickupIcon : public UBEInventoryItemFragment
{
	GENERATED_BODY()

public:
	UInventoryFragment_PickupIcon();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
	TObjectPtr<USkeletalMesh> SkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
	FLinearColor PadColor;
};

// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "CoreMinimal.h"
#include "Ability/Abilities/BEGameplayAbility.h"

#include "BEGameplayAbility_FromEquipment.generated.h"

class UBEEquipmentInstance;
class UBEInventoryItemInstance;

/**
 * UBEGameplayAbility_FromEquipment
 *
 * An ability granted by and associated with an equipment instance
 */
UCLASS()
class UBEGameplayAbility_FromEquipment : public UBEGameplayAbility
{
	GENERATED_BODY()

public:

	UBEGameplayAbility_FromEquipment(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category="BE|Ability")
	UBEEquipmentInstance* GetAssociatedEquipment() const;

	UFUNCTION(BlueprintCallable, Category = "BE|Ability")
	UBEInventoryItemInstance* GetAssociatedItem() const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
#endif

};

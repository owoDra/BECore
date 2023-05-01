// Copyright Eigi Chin

#pragma once

#include "Ability/BEGameplayAbility.h"

#include "Internationalization/Text.h"
#include "UObject/UObjectGlobals.h"

#include "BEGameplayAbility_FromEquipment.generated.h"

class UBEItemData;
class UBEEquipmentInstance;


/**
 * UBEGameplayAbility_FromEquipment
 *
 * Equipment とともに使用することが想定された GameplayAbility。
 * Equipment と共に使用しないと正常に動作しない。
 */
UCLASS()
class BECORE_API UBEGameplayAbility_FromEquipment : public UBEGameplayAbility
{
	GENERATED_BODY()

public:
	UBEGameplayAbility_FromEquipment(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	UBEEquipmentInstance* GetAssociatedEquipment() const;

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	const UBEItemData* GetAssociatedItem() const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
#endif
};

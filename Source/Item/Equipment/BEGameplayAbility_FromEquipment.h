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
 * Equipment �ƂƂ��Ɏg�p���邱�Ƃ��z�肳�ꂽ GameplayAbility�B
 * Equipment �Ƌ��Ɏg�p���Ȃ��Ɛ���ɓ��삵�Ȃ��B
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

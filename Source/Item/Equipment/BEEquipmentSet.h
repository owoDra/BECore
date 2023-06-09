// Copyright Eigi Chin

#pragma once

#include "Engine/DataAsset.h"

#include "Containers/Array.h"
#include "Internationalization/Text.h"
#include "UObject/ObjectPtr.h"
#include "UObject/UObjectGlobals.h"

#include "BEEquipmentSet.generated.h"

class UBEItemData;
struct FGameplayTag;


/**
 * FBEEquipmentSetEntry
 *
 * EquipmentSet 内で 追加する ItemData と Slot を定義するのに用いる
 */
USTRUCT(BlueprintType)
struct FBEEquipmentSetEntry
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Equipment", meta = (Categories = "Equipment.Slot"))
	FGameplayTag SlotTag;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<const UBEItemData> ItemData = nullptr;
};



/**
 * UBEEquipmentSet
 * 
 * EquipmentComponent に Equipment をまとめて追加するために使用されるデータアセット
 */
UCLASS(BlueprintType, Const, Abstract)
class UBEEquipmentSet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UBEEquipmentSet();
	
#if WITH_EDITOR //~UObject interface
	virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
#endif //~End of UObject interface


public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	TArray<FBEEquipmentSetEntry> Entries;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment", meta = (Categories = "Equipment.Slot"))
	FGameplayTag DefaultActiveSlotTag;
};

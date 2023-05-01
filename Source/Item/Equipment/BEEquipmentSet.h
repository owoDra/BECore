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
 * EquipmentSet ���� �ǉ����� ItemData �� Slot ���`����̂ɗp����
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
 * EquipmentComponent �� Equipment ���܂Ƃ߂Ēǉ����邽�߂Ɏg�p�����f�[�^�A�Z�b�g
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

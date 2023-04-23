// Copyright Eigi Chin

#pragma once

#include "BEItemData.h"

#include "UObject/UObjectGlobals.h"

#include "BEItemDataFragment_Category.generated.h"

struct FGameplayTagContainer;


UCLASS()
class UBEItemDataFragment_Category : public UBEItemDataFragment
{
	GENERATED_BODY()

public:
	// �����w�肵�Ȃ��ꍇ�͑S�ẴJ�e�S���[�Ɋi�[�\
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (Categories = "Inventory.Category"))
	FGameplayTagContainer AllowedCategoryTags;
};

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
	// 何も指定しない場合は全てのカテゴリーに格納可能
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (Categories = "Inventory.Category"))
	FGameplayTagContainer AllowedCategoryTags;
};

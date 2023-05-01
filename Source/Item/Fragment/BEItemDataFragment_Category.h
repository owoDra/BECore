// Copyright Eigi Chin

#pragma once

#include "Item/BEItemData.h"

#include "UObject/UObjectGlobals.h"

#include "BEItemDataFragment_Category.generated.h"

struct FGameplayTagContainer;


UCLASS()
class UBEItemDataFragment_Category : public UBEItemDataFragment
{
	GENERATED_BODY()

public:
	// Inventory に追加する際に追加可能なカテゴリーを定義する
	// 何も指定しなければすべてのカテゴリーに追加可能

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (Categories = "Inventory.Category"))
	FGameplayTagContainer AllowedCategoryTags;
};

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
	// Inventory �ɒǉ�����ۂɒǉ��\�ȃJ�e�S���[���`����
	// �����w�肵�Ȃ���΂��ׂẴJ�e�S���[�ɒǉ��\

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (Categories = "Inventory.Category"))
	FGameplayTagContainer AllowedCategoryTags;
};

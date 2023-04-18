// Copyright Eigi Chin

#pragma once

#include "UObject/Object.h"

#include "Containers/Array.h"
#include "Internationalization/Text.h"
#include "Templates/SubclassOf.h"
#include "UObject/ObjectPtr.h"
#include "UObject/UObjectGlobals.h"

#include "BEInventoryItemDefinition.generated.h"

class UBEInventoryItemFragment;
struct FGameplayTag;


/**
 * UBEInventoryItemDefinition
 * 
 * Item ���`����B
 * Item �̖��O��ڍׂȂǂ̂��炩���ߌ��߂�ꂽ���Ȃǂ�ۗL����B
 * Inventory XXX Component �� Item ��o�^����ۂɎg�p����B
 */
UCLASS(Blueprintable, Const, Abstract)
class UBEInventoryItemDefinition : public UObject
{
	GENERATED_BODY()

public:
	UBEInventoryItemDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Item")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Item", meta = (Categories = "Inventory.Category"))
	FGameplayTag CategoryTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Item", meta = (ClampMin = "1"))
	int32 MaxStack = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Inventory Item", Instanced)
	TArray<TObjectPtr<UBEInventoryItemFragment>> Fragments;

public:
	const UBEInventoryItemFragment* FindFragmentByClass(TSubclassOf<UBEInventoryItemFragment> FragmentClass) const;

	bool IsDefinitionValid() const;
};

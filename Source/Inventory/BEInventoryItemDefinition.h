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
 * Item を定義する。
 * Item の名前や詳細などのあらかじめ決められた情報などを保有する。
 * Inventory XXX Component に Item を登録する際に使用する。
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

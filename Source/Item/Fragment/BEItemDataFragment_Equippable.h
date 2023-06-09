// Copyright Eigi Chin

#pragma once

#include "Item/BEItemData.h"

#include "Containers/Array.h"
#include "Math/Transform.h"
#include "Templates/SubclassOf.h"
#include "UObject/NameTypes.h"
#include "UObject/ObjectPtr.h"
#include "UObject/UObjectGlobals.h"

#include "BEItemDataFragment_Equippable.generated.h"

class UBEAbilitySet;
class UBEEquipmentInstance;
struct FGameplayTagContainer;


/**
 * UBEItemDataFragment_Equippable
 *
 * Item が装備可能かどうかを識別するために使用。
 * Item を装備したときの情報が含まれる。
 */
UCLASS()
class UBEItemDataFragment_Equippable : public UBEItemDataFragment
{
	GENERATED_BODY()

public:
	// 装備を追加可能な Slot を指定する
	// 何も設定しない場合はすべての Slot に追加可能 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (Categories = "Equipment.Slot"))
	FGameplayTagContainer AllowedSlotTags;

	// 装備品の性能や状態を管理するためのクラス
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TSubclassOf<UBEEquipmentInstance> InstanceType;

	// 装備時に装備したPawnに付与するアビリティセット
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TArray<TObjectPtr<const UBEAbilitySet>> AbilitySetsToGrantOnEquip;

	// アクティブ時に装備したPawnに付与するアビリティセット
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TArray<TObjectPtr<const UBEAbilitySet>> AbilitySetsToGrantOnActive;
};

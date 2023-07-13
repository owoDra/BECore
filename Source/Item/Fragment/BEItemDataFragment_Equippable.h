// Copyright owoDra

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
struct FGameplayTag;
struct FGameplayTagContainer;
struct FBEEquipmentMeshToSpawn;


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
	// 設定しない場合はデフォルトの UBEEquipmentInstance をもとに作成する。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TSubclassOf<UBEEquipmentInstance> InstanceType;

	// 装備時に装備したPawnに付与するアビリティセット
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TArray<TObjectPtr<const UBEAbilitySet>> AbilitySetsToGrantOnEquip;

	// アクティブ時に装備したPawnに付与するアビリティセット
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TArray<TObjectPtr<const UBEAbilitySet>> AbilitySetsToGrantOnActive;

	// この Equipment の初期 StatTag
	// Equipment が作成された際の初期化処理に使用する
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TMap<FGameplayTag, int32> InitialEquipmentStats;

	// Equipment が Active になったときにスポーンする
	// 装備品の見た目となる SkeletalMesh の定義
	// 設定しない場合は何もスポーンしない。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TArray<FBEEquipmentMeshToSpawn> MeshesToSpawn;

	// Equipment が Active になったときに Pawn の FPP Mesh に
	// 適応する AnimLayer のクラス。
	// 設定しない場合は何も適応されない。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TSubclassOf<UAnimInstance> AnimLayerToApplyToFPP;

	// Equipment が Active になったときに Pawn の TPP Mesh に
	// 適応する AnimLayer のクラス。
	// 設定しない場合は何も適応されない。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TSubclassOf<UAnimInstance> AnimLayerToApplyToTPP;
};

// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "Components/ControllerComponent.h"

#include "Inventory/BEInventoryItemInstance.h"

#include "Containers/Array.h"
#include "HAL/Platform.h"
#include "UObject/ObjectPtr.h"
#include "UObject/UObjectGlobals.h"

#include "BEQuickBarComponent.generated.h"

class UBEInventoryItemInstance;
class UBEEquipmentInstance;
class UBEEquipmentManagerComponent;
class AActor;
class UObject;
struct FFrame;


// =====================================
//   構造体
// =====================================

USTRUCT(BlueprintType)
struct FBEQuickBarSlotsChangedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = Inventory)
		TObjectPtr<AActor> Owner = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = Inventory)
		TArray<TObjectPtr<UBEInventoryItemInstance>> Slots;
};


USTRUCT(BlueprintType)
struct FBEQuickBarActiveIndexChangedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = Inventory)
		TObjectPtr<AActor> Owner = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = Inventory)
		int32 ActiveIndex = 0;
};


// =====================================
//   クラス
// =====================================

/*
* UBEQuickBarComponent
* 
*/
UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class UBEQuickBarComponent : public UControllerComponent
{
	GENERATED_BODY()

		// =====================================
		//   初期化
		// =====================================
public:
	UBEQuickBarComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;


	// =====================================
	//  ユーティリティ
	// =====================================
public:
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Quick Bar")
		TArray<UBEInventoryItemInstance*> GetSlots() const { return Slots; };

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Quick Bar")
		int32 GetActiveSlot() const { return ActiveSlot; };

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Quick Bar")
		UBEInventoryItemInstance* GetActiveSlotItem() const { return Slots.IsValidIndex(ActiveSlot) ? Slots[ActiveSlot] : nullptr; };

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Quick Bar")
		int32 GetNextFreeItemSlot() const;

	UFUNCTION(BlueprintCallable, Category = "BE")
		void CycleActiveSlotForward();

	UFUNCTION(BlueprintCallable, Category = "BE")
		void CycleActiveSlotBackward();

private:
	UBEEquipmentManagerComponent* FindEquipmentManager() const;

	bool DoseEquipWhenAdded(UBEInventoryItemInstance* Item);

	bool DidEquipedWhenAdded(int32 SlotIndex);


	// =====================================
	//  スロット
	// =====================================
public:
	/*
	* クイックバーのアクティブスロットを変更する
	* 
	* 設定するスロットにすでにアクティブなアイテムがある場合は何も起こらない
	*/
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Quick Bar")
		void SetActiveSlot(int32 NewIndex);

	/*
	* クイックバーの指定したスロットにアイテムを登録する
	* 
	* すでにアイテムがある場合は先に古いアイテムを削除してから追加される
	*/
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Quick Bar")
		void AddItemToSlot(int32 SlotIndex, UBEInventoryItemInstance* Item);

	/*
	* クイックバーの指定したスロットのアイテムを削除する
	*/
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Quick Bar")
		UBEInventoryItemInstance* RemoveItemFromSlot(int32 SlotIndex);

private:
	UBEEquipmentInstance* EquipItem(UBEInventoryItemInstance* Item);

	void UnequipItem(UBEEquipmentInstance* Item);

protected:
	/*
	* クイックバーの最大スロット数
	* デフォルトは6。
	* 0:メイン武器, 1:サブ武器, 2:メインアビリティ, 3:サブアビリティ, 4:ウルト, 5:アイテム
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quick Bar")
		int32 NumSlots = 6;

	UFUNCTION()
		void OnRep_Slots();

	UFUNCTION()
		void OnRep_ActiveSlot();

private:
	/*
	* クイックバーのスロット
	*/
	UPROPERTY(ReplicatedUsing = OnRep_Slots)
		TArray<TObjectPtr<UBEInventoryItemInstance>> Slots;

	/*
	* クイックバーの現在アクティブなスロット
	*/
	UPROPERTY(ReplicatedUsing = OnRep_ActiveSlot)
		int32 ActiveSlot = -1;

	/*
	* クイックバーの現在アクティブなスロットのアイテム
	* 
	* 武器などの構えて装備するタイプのアイテムで一つまでしか設定できない
	*/
	UPROPERTY()
		TObjectPtr<UBEEquipmentInstance> ActiveItem;

	/*
	* クイックバーに登録されたときにアクティブになるアイテム
	* 
	* タクティカルアビリティなどでGameplay Abilityを設定と同時に付与するアイテムが登録される
	*/
	UPROPERTY()
		TMap<int32 ,TObjectPtr<UBEEquipmentInstance>> ActivatedSubItems;
};

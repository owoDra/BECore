// Copyright Eigi Chin

#pragma once

#include "Net/Serialization/FastArraySerializer.h"
#include "Components/ActorComponent.h"

#include "Containers/Array.h"
#include "Containers/ArrayView.h"
#include "Containers/Map.h"
#include "Containers/Set.h"
#include "Containers/SparseArray.h"
#include "Containers/UnrealString.h"
#include "CoreTypes.h"
#include "Templates/SubclassOf.h"
#include "Templates/UnrealTemplate.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "NativeGameplayTags.h"

#include "BEInventoryStorageComponent.generated.h"

class UBEInventoryItemDefinition;
class UBEInventoryItemInstance;
class UBEInventoryStorageComponent;
class UObject;
struct FBEInventoryList;
struct FNetDeltaSerializeInfo;
struct FReplicationFlags;
struct FGameplayTag;
struct FGameplayTagContainer;

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Inventory_StackChanged);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Category);


/** 
 * FBEInventoryChangeMessage
 * 
 *  Inventory 内の Item に変更があったときのメッセージ
 */
USTRUCT(BlueprintType)
struct FBEInventoryChangeMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UActorComponent> InventoryOwner = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UBEInventoryItemInstance> Instance = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	FGameplayTag CategoryTag = FGameplayTag::EmptyTag;

	UPROPERTY(BlueprintReadOnly, Category= "Inventory")
	int32 NewCount = 0;

	UPROPERTY(BlueprintReadOnly, Category= "Inventory")
	int32 Delta = 0;
};


/**
 * FBEInventoryEntry
 *
 *  Inventory に追加された Item のデータ
 */
USTRUCT(BlueprintType)
struct FBEInventoryEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FBEInventoryEntry()
	{}

	FString GetDebugString() const;

	int32 GetMaxStack() const;

	FGameplayTag GetCategoryTag() const;

private:
	friend FBEInventoryList;
	friend UBEInventoryStorageComponent;

	UPROPERTY()
	TObjectPtr<UBEInventoryItemInstance> Instance = nullptr;

	UPROPERTY()
	int32 StackCount = 0;

	UPROPERTY(NotReplicated)
	int32 LastObservedCount = INDEX_NONE;
};


/**
 * FBEInventoryList
 *
 *  Inventory 内の Item のリスト
 */
USTRUCT(BlueprintType)
struct FBEInventoryList : public FFastArraySerializer
{
	GENERATED_BODY()

	FBEInventoryList()
		: OwnerComponent(nullptr)
	{
	}

	FBEInventoryList(UActorComponent* InOwnerComponent)
		: OwnerComponent(InOwnerComponent)
	{
	}

private:
	friend UBEInventoryStorageComponent;

public:
	//~FFastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~End of FFastArraySerializer contract

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FBEInventoryEntry, FBEInventoryList>(Entries, DeltaParms, *this);
	}

public:
	UBEInventoryItemInstance* AddEntry(TSubclassOf<UBEInventoryItemDefinition> ItemDef, int32 StackCount);
	void RemoveEntry(UBEInventoryItemInstance* Instance, int32 StackCount);

	TArray<UBEInventoryItemInstance*> GetAllItems() const;
	TArray<UBEInventoryItemInstance*> GetCategoryItems(const FGameplayTagContainer& CategoryTags) const;

private:
	void BroadcastChangeMessage(FBEInventoryEntry& Entry, int32 OldCount, int32 NewCount);

private:
	UPROPERTY()
	TArray<FBEInventoryEntry> Entries;

	UPROPERTY(NotReplicated)
	TObjectPtr<UActorComponent> OwnerComponent;
};

template<>
struct TStructOpsTypeTraits<FBEInventoryList> : public TStructOpsTypeTraitsBase2<FBEInventoryList>
{
	enum { WithNetDeltaSerializer = true };
};


/**
 * UBEInventoryStorageComponent
 * 
 *  所持する Item を管理するための Inventory。
 */
UCLASS(BlueprintType)
class BECORE_API UBEInventoryStorageComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBEInventoryStorageComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UObject interface
	virtual void ReadyForReplication() override;
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	//~End of UObject interface

public:
	/**
	 * CanAddItemDefinition
	 *
	 *  Inventory に Item Definition を使用して Item を追加可能かどうかを返す
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory")
	bool CanAddItemDefinition(TSubclassOf<UBEInventoryItemDefinition> ItemDef, int32 StackCount = 1);

	/**
	 * AddItemByDefinition
	 *
	 *  Inventory に Item Definition を使用して Item を追加する。
	 *  実行にはサーバー権限が必要。
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category= "Inventory")
	UBEInventoryItemInstance* AddItemByDefinition(TSubclassOf<UBEInventoryItemDefinition> ItemDef, int32 StackCount = 1);

	/**
	 * RemoveItemByDefinition
	 *
	 *  Inventory の Item Definition に該当する Item を削除する。
	 *  実行にはサーバー権限が必要。
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory")
	void RemoveItemByDefinition(TSubclassOf<UBEInventoryItemDefinition> ItemDef, int32 StackCount = -1);

	/**
	 * RemoveItem
	 *
	 *  Inventory の Item を削除する。
	 *  実行にはサーバー権限が必要。
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category= "Inventory")
	void RemoveItem(UBEInventoryItemInstance* ItemInstance, int32 StackCount = -1);


public:
	/**
	 * GetAllItems
	 *
	 *  Inventory 内の Item を全て取得する
	 */
	UFUNCTION(BlueprintCallable, Category= "Inventory", BlueprintPure = false)
	TArray<UBEInventoryItemInstance*> GetAllItems() const;

	/**
	 * GetCategoryItems
	 *
	 *  Inventory 内の指定したカテゴリーの Item を全て取得する
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory", BlueprintPure = false)
	TArray<UBEInventoryItemInstance*> GetCategoryItems(const FGameplayTagContainer& CategoryTags) const;

	/**
	 * FindItemByDefinition
	 *
	 *  Inventory 内の指定した Item Definition の Item を取得する
	 */
	UFUNCTION(BlueprintCallable, Category= "Inventory", BlueprintPure)
	UBEInventoryItemInstance* FindItemByDefinition(TSubclassOf<UBEInventoryItemDefinition> ItemDef) const;

	/**
	 * GetTotalItemCountByDefinition
	 *
	 *  Inventory 内の指定した Item Definition の Item の数を取得する
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory", BlueprintPure, meta = (DisplayName = "GetItemCount"))
	int32 GetTotalItemCountByDefinition(TSubclassOf<UBEInventoryItemDefinition> ItemDef) const;


private:
	UPROPERTY(Replicated)
	FBEInventoryList InventoryList;
};

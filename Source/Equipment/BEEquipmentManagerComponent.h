// Copyright Eigi Chin

#pragma once

#include "Components/PawnComponent.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "Ability/BEAbilitySet.h"

#include "Containers/Array.h"
#include "Containers/ArrayView.h"
#include "Containers/Map.h"
#include "Containers/Set.h"
#include "Containers/SparseArray.h"
#include "Containers/UnrealString.h"
#include "HAL/Platform.h"
#include "Templates/SubclassOf.h"
#include "Templates/UnrealTemplate.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "NativeGameplayTags.h"

#include "BEEquipmentManagerComponent.generated.h"

class UBEItemData;
class UBEEquipmentInstance;
class UBEEquipmentSlotData;
class UBEEquipmentManagerComponent;

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Equipment_SlotChange);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Equipment_ActiveSlotChange);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Equipment_Slot);


/**
 * FBEEquipmentSlotChangedMessage
 *
 * EquipmentManagerComponent に登録した Equipment が変更されたときのメッセージ
 */
USTRUCT(BlueprintType)
struct FBEEquipmentSlotChangedMessage
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UActorComponent> OwnerComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	FGameplayTag SlotTag = FGameplayTag::EmptyTag;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UBEItemData> ItemData = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UBEEquipmentInstance> Instance = nullptr;
};


/**
 * FBEEquipmentEntry
 *
 * EquipmentManagerComponent に登録した Equipment の情報
 */
USTRUCT(BlueprintType)
struct FBEEquipmentEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FBEEquipmentEntry()
	{
		Activated = false;
	}

private:
	friend FBEEquipmentList;
	friend UBEEquipmentManagerComponent;

	UPROPERTY()
	TObjectPtr<UBEItemData> ItemData = nullptr;

	UPROPERTY()
	TObjectPtr<UBEEquipmentInstance> Instance = nullptr;

	UPROPERTY()
	FGameplayTag SlotTag = FGameplayTag::EmptyTag;

	UPROPERTY()
	uint8 Activated : 1;

	// EquipmentManagerComponent に登録した際に付与される AbilitySet の GrantedHandles
	// データを保持するのはサーバー権限のみ
	UPROPERTY(NotReplicated)
	FBEAbilitySet_GrantedHandles GrantedHandles_Equip;

	// EquipmentManagerComponent で Active にしたときに付与される AbilitySet の GrantedHandles
	// データを保持するのはサーバー権限のみ
	UPROPERTY(NotReplicated)
	FBEAbilitySet_GrantedHandles GrantedHandles_Active;

public:
	FString GetDebugString() const;

	bool IsValid() const;
};


/**
 * FBEEquipmentList
 *
 * EquipmentManagerComponent に登録した Equipment のリスト
 */
USTRUCT(BlueprintType)
struct FBEEquipmentList : public FFastArraySerializer
{
	GENERATED_BODY()

	FBEEquipmentList()
		: OwnerComponent(nullptr)
	{
	}

	FBEEquipmentList(UActorComponent* InOwnerComponent)
		: OwnerComponent(InOwnerComponent)
	{
	}

private:
	friend UBEEquipmentManagerComponent;


public:
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FBEEquipmentEntry, FBEEquipmentList>(Entries, DeltaParms, *this);
	}

	//~FFastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~End of FFastArraySerializer contract

private:
	void BroadcastSlotChangeMessage(
		FGameplayTag SlotTag = FGameplayTag::EmptyTag,
		UBEItemData* ItemData = nullptr,
		UBEEquipmentInstance* Instance = nullptr);

	void BroadcastActiveSlotChangeMessage(
		FGameplayTag SlotTag = FGameplayTag::EmptyTag,
		UBEItemData* ItemData = nullptr,
		UBEEquipmentInstance* Instance = nullptr);


public:
	UBEEquipmentInstance* AddEntry(UBEItemData* ItemData, FGameplayTag SlotTag, UBEAbilitySystemComponent* ASC);
	void RemoveEntry(int32 Index, UBEAbilitySystemComponent* ASC);

	void ActivateEntry(int32 Index, UBEAbilitySystemComponent* ASC);
	void DeactivateEntry(int32 Index, UBEAbilitySystemComponent* ASC);


private:
	UPROPERTY()
	TArray<FBEEquipmentEntry> Entries;

	UPROPERTY(NotReplicated)
	TObjectPtr<UActorComponent> OwnerComponent;
};

template<>
struct TStructOpsTypeTraits<FBEEquipmentList> : public TStructOpsTypeTraitsBase2<FBEEquipmentList>
{
	enum { WithNetDeltaSerializer = true };
};


/**
 * UBEEquipmentManagerComponent
 *
 * Equipment を管理するためのコンポーネント
 */
UCLASS(BlueprintType, Const)
class UBEEquipmentManagerComponent : public UPawnComponent
{
	GENERATED_BODY()

	UBEEquipmentManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	//~UObject interface
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	//~End of UObject interface

	//~UActorComponent interface
	virtual void UninitializeComponent() override;
	virtual void ReadyForReplication() override;
	//~End of UActorComponent interface


private:
	UBEAbilitySystemComponent* GetAbilitySystemComponent() const;


public:
	/**
	 * AddEquipment
	 *
	 * 指定した Slot に ItemData を追加する。
	 * ItemData をもとに Equipment を作成する。
	 * 複数の ItemData を追加する場合は AddEquipments を使用してください。
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Equipment")
	bool AddEquipment(FGameplayTag SlotTag, UBEItemData* ItemData, bool ActivateImmediately = true);

	/**
	 * AddEquipments
	 *
	 * 指定した Slot に ItemData を追加する。
	 * 複数の ItemData をいっぺんに追加する。
	 * ActivateSlotTag に SlotTag を指定することで Active にすることができる。
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Equipment")
	void AddEquipments(const TMap<FGameplayTag, UBEItemData*>& ItemDatas, FGameplayTag ActivateSlotTag = FGameplayTag());

	/**
	 * RemoveEquipment
	 *
	 * 指定した Slot の Equipment を削除する。
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Equipment")
	bool RemoveEquipment(FGameplayTag SlotTag);

	/**
	 * RemoveAllEquipments
	 *
	 * つかされたすべての Equipment を削除する。
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Equipment")
	void RemoveAllEquipments();


public:
	/**
	 * SetActiveSlot
	 *
	 * Active な Slot を変更する
	 * 設定する Slot がすでに Active な場合は何も起こらない
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Equipment")
	void SetActiveSlot(FGameplayTag SlotTag);

private:
	UPROPERTY(Replicated)
	FBEEquipmentList EquipmentList;

	UPROPERTY(Transient)
	TSet<FGameplayTag> AddedSlotTags;
};

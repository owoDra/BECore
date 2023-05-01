// Copyright Eigi Chin

#pragma once

#include "Components/PawnComponent.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "Character/Component/BECharacterBasicComponent.h"
#include "Ability/BEAbilitySet.h"
#include "Item/Equipment/BEEquipmentSet.h"

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

#include "BECharacterEquipmentComponent.generated.h"

class UBEItemData;
class UBEEquipmentInstance;
class UBEEquipmentSlotData;
class UBECharacterEquipmentComponent;

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Equipment_SlotChange);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Equipment_ActiveSlotChange);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Equipment_Slot);


/**
 * FBEEquipmentSlotChangedMessage
 *
 * EquipmentManagerComponent �ɓo�^���� Equipment ���ύX���ꂽ�Ƃ��̃��b�Z�[�W
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
	TObjectPtr<const UBEItemData> ItemData = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UBEEquipmentInstance> Instance = nullptr;
};


/**
 * FBEEquipmentEntry
 *
 * EquipmentManagerComponent �ɓo�^���� Equipment �̏��
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
	friend UBECharacterEquipmentComponent;

	UPROPERTY()
	TObjectPtr<const UBEItemData> ItemData = nullptr;

	UPROPERTY()
	TObjectPtr<UBEEquipmentInstance> Instance = nullptr;

	UPROPERTY()
	FGameplayTag SlotTag = FGameplayTag::EmptyTag;

	UPROPERTY()
	uint8 Activated : 1;

	// EquipmentManagerComponent �ɓo�^�����ۂɕt�^����� AbilitySet �� GrantedHandles
	// �f�[�^��ێ�����̂̓T�[�o�[�����̂�
	UPROPERTY(NotReplicated)
	FBEAbilitySet_GrantedHandles GrantedHandles_Equip;

	// EquipmentManagerComponent �� Active �ɂ����Ƃ��ɕt�^����� AbilitySet �� GrantedHandles
	// �f�[�^��ێ�����̂̓T�[�o�[�����̂�
	UPROPERTY(NotReplicated)
	FBEAbilitySet_GrantedHandles GrantedHandles_Active;

public:
	FString GetDebugString() const;

	bool IsValid() const;
};


/**
 * FBEEquipmentList
 *
 * EquipmentManagerComponent �ɓo�^���� Equipment �̃��X�g
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
	friend UBECharacterEquipmentComponent;


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
		const UBEItemData* ItemData = nullptr,
		UBEEquipmentInstance* Instance = nullptr);

	void BroadcastActiveSlotChangeMessage(
		FGameplayTag SlotTag = FGameplayTag::EmptyTag,
		const UBEItemData* ItemData = nullptr,
		UBEEquipmentInstance* Instance = nullptr);


public:
	UBEEquipmentInstance* AddEntry(const UBEItemData* ItemData, FGameplayTag SlotTag, UBEAbilitySystemComponent* ASC);
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
 * UBECharacterEquipmentComponent
 *
 * Equipment ���Ǘ����邽�߂̃R���|�[�l���g
 */
UCLASS(BlueprintType, Const)
class BECORE_API UBECharacterEquipmentComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:
	UBECharacterEquipmentComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// ���̃R���|�[�l���g����������ۂ� FeatureName
	static const FName NAME_ActorFeatureName;

protected:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


public:
	//~ Begin IGameFrameworkInitStateInterface interface
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void CheckDefaultInitialization() override;
	//~ End IGameFrameworkInitStateInterface interface


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
	 * �w�肵�� Slot �� ItemData ��ǉ�����B
	 * ItemData �����Ƃ� Equipment ���쐬����B
	 * ������ ItemData ��ǉ�����ꍇ�� AddEquipments ���g�p���Ă��������B
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Equipment", meta = (GameplayTagFilter = "Equipment.Slot"))
	bool AddEquipment(FGameplayTag SlotTag, const UBEItemData* ItemData, bool ActivateImmediately = true);

	/**
	 * AddEquipments
	 *
	 * �w�肵�� Slot �� ItemData ��ǉ�����B
	 * ������ ItemData �������؂�ɒǉ�����B
	 * ActivateSlotTag �� SlotTag ���w�肷�邱�Ƃ� Active �ɂ��邱�Ƃ��ł���B
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Equipment", meta = (GameplayTagFilter = "Equipment.Slot"))
	void AddEquipments(const TArray<FBEEquipmentSetEntry>& Entries, FGameplayTag ActivateSlotTag = FGameplayTag());

	/**
	 * RemoveEquipment
	 *
	 * �w�肵�� Slot �� Equipment ���폜����B
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Equipment", meta = (GameplayTagFilter = "Equipment.Slot"))
	bool RemoveEquipment(FGameplayTag SlotTag);

	/**
	 * RemoveAllEquipments
	 *
	 * �����ꂽ���ׂĂ� Equipment ���폜����B
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Equipment")
	void RemoveAllEquipments();


public:
	/**
	 * SetActiveSlot
	 *
	 * Active �� Slot ��ύX����
	 * �ݒ肷�� Slot �����ł� Active �ȏꍇ�͉����N����Ȃ�
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Equipment", meta = (GameplayTagFilter = "Equipment.Slot"))
	void SetActiveSlot(FGameplayTag SlotTag);

	/**
	 * GetActiveSlotInfo
	 *
	 * Active �� Slot �� ItemData ����� Equipment�ASlotTag ��Ԃ�
	 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	bool GetActiveSlotInfo(FBEEquipmentSlotChangedMessage& SlotInfo);

	/**
	 * GetSlotInfo
	 *
	 * �w�肵�� Slot �� ItemData ����� Equipment�ASlotTag ��Ԃ�
	 */
	UFUNCTION(BlueprintCallable, Category = "Equipment", meta = (GameplayTagFilter = "Equipment.Slot"))
	bool GetSlotInfo(FGameplayTag SlotTag, FBEEquipmentSlotChangedMessage& SlotInfo);

private:
	UPROPERTY(Replicated)
	FBEEquipmentList EquipmentList;

	UPROPERTY(Transient)
	TSet<FGameplayTag> AddedSlotTags;
};

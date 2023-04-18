// Copyright Epic Games, Inc. All Rights Reserved.
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

#include "BEEquipmentManagerComponent.generated.h"

class UObject;
class UActorComponent;
class UBEEquipmentDefinition;
class UBEEquipmentInstance;
class UBEAbilitySystemComponent;
class UBEEquipmentManagerComponent;
struct FFrame;
struct FNetDeltaSerializeInfo;
struct FReplicationFlags;
struct FBEEquipmentList;


/** A single piece of applied equipment */
USTRUCT(BlueprintType)
struct FBEAppliedEquipmentEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FBEAppliedEquipmentEntry()
	{}

	FString GetDebugString() const;

private:
	friend FBEEquipmentList;
	friend UBEEquipmentManagerComponent;

	// The equipment class that got equipped
	UPROPERTY()
	TSubclassOf<UBEEquipmentDefinition> EquipmentDefinition;

	UPROPERTY()
	TObjectPtr<UBEEquipmentInstance> Instance = nullptr;

	// Authority-only list of granted handles
	UPROPERTY(NotReplicated)
	FBEAbilitySet_GrantedHandles GrantedHandles;
};

/** List of applied equipment */
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

public:
	//~FFastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~End of FFastArraySerializer contract

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FBEAppliedEquipmentEntry, FBEEquipmentList>(Entries, DeltaParms, *this);
	}

	UBEEquipmentInstance* AddEntry(TSubclassOf<UBEEquipmentDefinition> EquipmentDefinition);
	void RemoveEntry(UBEEquipmentInstance* Instance);

private:
	UBEAbilitySystemComponent* GetAbilitySystemComponent() const;

	friend UBEEquipmentManagerComponent;

private:
	// Replicated list of equipment entries
	UPROPERTY()
	TArray<FBEAppliedEquipmentEntry> Entries;

	UPROPERTY(NotReplicated)
	TObjectPtr<UActorComponent> OwnerComponent;
};

template<>
struct TStructOpsTypeTraits<FBEEquipmentList> : public TStructOpsTypeTraitsBase2<FBEEquipmentList>
{
	enum { WithNetDeltaSerializer = true };
};



/**
 * Manages equipment applied to a pawn
 */
UCLASS(BlueprintType, Const)
class UBEEquipmentManagerComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	UBEEquipmentManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	UBEEquipmentInstance* EquipItem(TSubclassOf<UBEEquipmentDefinition> EquipmentDefinition);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void UnequipItem(UBEEquipmentInstance* ItemInstance);

	//~UObject interface
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	//~End of UObject interface

	//~UActorComponent interface
	//virtual void EndPlay() override;
	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;
	virtual void ReadyForReplication() override;
	//~End of UActorComponent interface

	/** Returns the first equipped instance of a given type, or nullptr if none are found */
	UFUNCTION(BlueprintCallable, BlueprintPure)
	UBEEquipmentInstance* GetFirstInstanceOfType(TSubclassOf<UBEEquipmentInstance> InstanceType);

 	/** Returns all equipped instances of a given type, or an empty array if none are found */
 	UFUNCTION(BlueprintCallable, BlueprintPure)
	TArray<UBEEquipmentInstance*> GetEquipmentInstancesOfType(TSubclassOf<UBEEquipmentInstance> InstanceType) const;

	template <typename T>
	T* GetFirstInstanceOfType()
	{
		return (T*)GetFirstInstanceOfType(T::StaticClass());
	}

private:
	UPROPERTY(Replicated)
	FBEEquipmentList EquipmentList;
};

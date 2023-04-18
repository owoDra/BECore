// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "Net/Serialization/FastArraySerializer.h"

#include "BEVerbMessage.h"

#include "Containers/Array.h"
#include "Containers/ArrayView.h"
#include "Containers/Map.h"
#include "Containers/Set.h"
#include "Containers/SparseArray.h"
#include "Containers/UnrealString.h"
#include "HAL/Platform.h"
#include "Templates/UnrealTemplate.h"
#include "UObject/Class.h"

#include "BEVerbMessageReplication.generated.h"

class UObject;
struct FNetDeltaSerializeInfo;
struct FBEVerbMessageReplication;


/**
 * Represents one verb message
 */
USTRUCT(BlueprintType)
struct FBEVerbMessageReplicationEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FBEVerbMessageReplicationEntry()
	{}

	FBEVerbMessageReplicationEntry(const FBEVerbMessage& InMessage)
		: Message(InMessage)
	{
	}

	FString GetDebugString() const;

private:
	friend FBEVerbMessageReplication;

	UPROPERTY()
	FBEVerbMessage Message;
};

/** Container of verb messages to replicate */
USTRUCT(BlueprintType)
struct FBEVerbMessageReplication : public FFastArraySerializer
{
	GENERATED_BODY()

	FBEVerbMessageReplication()
	{
	}

public:
	void SetOwner(UObject* InOwner) { Owner = InOwner; }

	// Broadcasts a message from server to clients
	void AddMessage(const FBEVerbMessage& Message);

	//~FFastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~End of FFastArraySerializer contract

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FBEVerbMessageReplicationEntry, FBEVerbMessageReplication>(CurrentMessages, DeltaParms, *this);
	}

private:
	void RebroadcastMessage(const FBEVerbMessage& Message);

private:
	// Replicated list of gameplay tag stacks
	UPROPERTY()
	TArray<FBEVerbMessageReplicationEntry> CurrentMessages;
	
	// Owner (for a route to a world)
	UPROPERTY()
	TObjectPtr<UObject> Owner = nullptr;
};

template<>
struct TStructOpsTypeTraits<FBEVerbMessageReplication> : public TStructOpsTypeTraitsBase2<FBEVerbMessageReplication>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};

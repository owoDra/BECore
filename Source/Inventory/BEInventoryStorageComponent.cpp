// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BEInventoryStorageComponent.h"

#include "BEInventoryItemDefinition.h"
#include "BEInventoryItemInstance.h"
#include "Fragment/BEInventoryItemFragment.h"
#include "BEGameplayTags.h"

#include "Engine/ActorChannel.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Misc/AssertionMacros.h"
#include "Net/UnrealNetwork.h"
#include "UObject/NameTypes.h"
#include "UObject/Object.h"
#include "UObject/ObjectPtr.h"
#include "UObject/UObjectBaseUtility.h"
#include "GameplayTagContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEInventoryStorageComponent)

class FLifetimeProperty;
struct FReplicationFlags;

UE_DEFINE_GAMEPLAY_TAG(TAG_Message_Inventory_StackChanged, "Message.Inventory.StackChanged")
UE_DEFINE_GAMEPLAY_TAG(TAG_Inventory_Category, "Inventory.Category")


//////////////////////////////////////////////////////////////////////
// FBEInventoryEntry

FString FBEInventoryEntry::GetDebugString() const
{
	TSubclassOf<UBEInventoryItemDefinition> ItemDef;
	if (Instance != nullptr)
	{
		ItemDef = Instance->GetItemDef();
	}
	return FString::Printf(TEXT("(Def:%s) %s (%dx)"), *GetNameSafe(ItemDef), *GetNameSafe(Instance), StackCount);
}

int32 FBEInventoryEntry::GetMaxStack() const
{
	if (Instance != nullptr)
	{
		if (const UBEInventoryItemDefinition* ItemCDO = GetDefault<UBEInventoryItemDefinition>(Instance->GetItemDef()))
		{
			return ItemCDO->MaxStack;
		}
	}
	return 0;
}

FGameplayTag FBEInventoryEntry::GetCategoryTag() const
{
	if (Instance != nullptr)
	{
		if (const UBEInventoryItemDefinition* ItemCD0 = GetDefault<UBEInventoryItemDefinition>(Instance->GetItemDef()))
		{
			return ItemCD0->CategoryTag;
		}
	}
	return FGameplayTag::EmptyTag;
}


//////////////////////////////////////////////////////////////////////
// FBEInventoryList

void FBEInventoryList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (int32 Index : RemovedIndices)
	{
		FBEInventoryEntry& Stack = Entries[Index];
		BroadcastChangeMessage(Stack, /*OldCount=*/ Stack.StackCount, /*NewCount=*/ 0);
		Stack.LastObservedCount = 0;
	}
}

void FBEInventoryList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		FBEInventoryEntry& Stack = Entries[Index];
		BroadcastChangeMessage(Stack, /*OldCount=*/ 0, /*NewCount=*/ Stack.StackCount);
		Stack.LastObservedCount = Stack.StackCount;
	}
}

void FBEInventoryList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		FBEInventoryEntry& Stack = Entries[Index];
		check(Stack.LastObservedCount != INDEX_NONE);
		BroadcastChangeMessage(Stack, /*OldCount=*/ Stack.LastObservedCount, /*NewCount=*/ Stack.StackCount);
		Stack.LastObservedCount = Stack.StackCount;
	}
}

void FBEInventoryList::BroadcastChangeMessage(FBEInventoryEntry& Entry, int32 OldCount, int32 NewCount)
{
	FBEInventoryChangeMessage Message;
	Message.InventoryOwner = OwnerComponent;
	Message.Instance = Entry.Instance;
	Message.CategoryTag = Entry.GetCategoryTag();
	Message.NewCount = NewCount;
	Message.Delta = NewCount - OldCount;

	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(OwnerComponent->GetWorld());
	MessageSystem.BroadcastMessage(TAG_Message_Inventory_StackChanged, Message);
}


UBEInventoryItemInstance* FBEInventoryList::AddEntry(TSubclassOf<UBEInventoryItemDefinition> ItemDef, int32 StackCount)
{
	check(ItemDef != nullptr);
	check(OwnerComponent);
	AActor* OwningActor = OwnerComponent->GetOwner();
	check(OwningActor->HasAuthority());

	UBEInventoryItemInstance* Result = nullptr;

	// 既に同じ Item があるか検証する。ある場合は StackCount のみ更新する。
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FBEInventoryEntry& Entry = *EntryIt;
		if (Entry.Instance->GetItemDef() == ItemDef)
		{
			Entry.StackCount = Entry.StackCount + StackCount;
			Result = Entry.Instance;
			MarkItemDirty(Entry);

			return Result;
		}
	}

	// 既に同じ Item が追加してない場合に新しく作成する。
	FBEInventoryEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.Instance = NewObject<UBEInventoryItemInstance>(OwningActor);
	NewEntry.Instance->SetItemDef(ItemDef);
	for (UBEInventoryItemFragment* Fragment : GetDefault<UBEInventoryItemDefinition>(ItemDef)->Fragments)
	{
		if (Fragment != nullptr)
		{
			Fragment->OnInstanceCreated(NewEntry.Instance);
		}
	}
	NewEntry.StackCount = StackCount;
	Result = NewEntry.Instance;

	MarkItemDirty(NewEntry);

	return Result;
}

void FBEInventoryList::RemoveEntry(UBEInventoryItemInstance* Instance, int32 StackCount)
{
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FBEInventoryEntry& Entry = *EntryIt;
		if (Entry.Instance == Instance)
		{
			// 削除する Item の所持数が削除すべき数以下、または StackCount が -1 の場合に配列から削除する。
			if ((Entry.StackCount <= StackCount) || (StackCount == -1))
			{
				EntryIt.RemoveCurrent();
				MarkArrayDirty();
			}

			// 削除する Item の所持数が削除すべき数より多い場合は所持数を更新する。
			else
			{
				Entry.StackCount -= StackCount;
				MarkItemDirty(Entry);
			}
		}
	}
}


TArray<UBEInventoryItemInstance*> FBEInventoryList::GetAllItems() const
{
	TArray<UBEInventoryItemInstance*> Results;
	Results.Reserve(Entries.Num());
	for (const FBEInventoryEntry& Entry : Entries)
	{
		if (Entry.Instance != nullptr)
		{
			Results.Add(Entry.Instance);
		}
	}
	return Results;
}

TArray<UBEInventoryItemInstance*> FBEInventoryList::GetCategoryItems(const FGameplayTagContainer& CategoryTags) const
{
	TArray<UBEInventoryItemInstance*> Results;
	Results.Reserve(Entries.Num());
	for (const FBEInventoryEntry& Entry : Entries)
	{
		if (CategoryTags.HasTag(Entry.GetCategoryTag()))
		{
			Results.Add(Entry.Instance);
		}
	}
	return Results;
}

//////////////////////////////////////////////////////////////////////
// UBEInventoryStorageComponent

UBEInventoryStorageComponent::UBEInventoryStorageComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, InventoryList(this)
{
	SetIsReplicatedByDefault(true);
}

void UBEInventoryStorageComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, InventoryList);
}

void UBEInventoryStorageComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	// Register existing UBEInventoryItemInstance
	if (IsUsingRegisteredSubObjectList())
	{
		for (const FBEInventoryEntry& Entry : InventoryList.Entries)
		{
			UBEInventoryItemInstance* Instance = Entry.Instance;

			if (IsValid(Instance))
			{
				AddReplicatedSubObject(Instance);
			}
		}
	}
}

bool UBEInventoryStorageComponent::ReplicateSubobjects(UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (FBEInventoryEntry& Entry : InventoryList.Entries)
	{
		UBEInventoryItemInstance* Instance = Entry.Instance;

		if (Instance && IsValid(Instance))
		{
			WroteSomething |= Channel->ReplicateSubobject(Instance, *Bunch, *RepFlags);
		}
	}

	return WroteSomething;
}


bool UBEInventoryStorageComponent::CanAddItemDefinition(TSubclassOf<UBEInventoryItemDefinition> ItemDef, int32 StackCount)
{
	if (ItemDef == nullptr)
	{
		return false;
	}

	const UBEInventoryItemDefinition* ItemDefCDO = GetDefault<UBEInventoryItemDefinition>(ItemDef);
	if (!ItemDefCDO->IsDefinitionValid())
	{
		return false;
	}

	for (const FBEInventoryEntry& Entry : InventoryList.Entries)
	{
		UBEInventoryItemInstance* Instance = Entry.Instance;

		if (IsValid(Instance))
		{
			if (Instance->GetItemDef() == ItemDef)
			{
				if (ItemDefCDO->MaxStack < (Entry.StackCount + StackCount))
				{
					return false;
				}
			}
		}
	}

	return true;
}

UBEInventoryItemInstance* UBEInventoryStorageComponent::AddItemByDefinition(TSubclassOf<UBEInventoryItemDefinition> ItemDef, int32 StackCount)
{
	UBEInventoryItemInstance* Result = nullptr;
	if (ItemDef != nullptr)
	{
		Result = InventoryList.AddEntry(ItemDef, StackCount);

		if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && Result)
		{
			AddReplicatedSubObject(Result);
		}
	}
	return Result;
}

void UBEInventoryStorageComponent::RemoveItemByDefinition(TSubclassOf<UBEInventoryItemDefinition> ItemDef, int32 StackCount)
{
	for (const FBEInventoryEntry& Entry : InventoryList.Entries)
	{
		UBEInventoryItemInstance* Instance = Entry.Instance;

		if (IsValid(Instance))
		{
			if (Instance->GetItemDef() == ItemDef)
			{
				RemoveItem(Instance, StackCount);
				return;
			}
		}
	}
}

void UBEInventoryStorageComponent::RemoveItem(UBEInventoryItemInstance* ItemInstance, int32 StackCount)
{
	InventoryList.RemoveEntry(ItemInstance, StackCount);

	if (ItemInstance && IsUsingRegisteredSubObjectList())
	{
		RemoveReplicatedSubObject(ItemInstance);
	}
}


TArray<UBEInventoryItemInstance*> UBEInventoryStorageComponent::GetAllItems() const
{
	return InventoryList.GetAllItems();
}

TArray<UBEInventoryItemInstance*> UBEInventoryStorageComponent::GetCategoryItems(const FGameplayTagContainer& CategoryTags) const
{
	return InventoryList.GetCategoryItems();
}

UBEInventoryItemInstance* UBEInventoryStorageComponent::FindItemByDefinition(TSubclassOf<UBEInventoryItemDefinition> ItemDef) const
{
	for (const FBEInventoryEntry& Entry : InventoryList.Entries)
	{
		UBEInventoryItemInstance* Instance = Entry.Instance;

		if (IsValid(Instance))
		{
			if (Instance->GetItemDef() == ItemDef)
			{
				return Instance;
			}
		}
	}

	return nullptr;
}

int32 UBEInventoryStorageComponent::GetTotalItemCountByDefinition(TSubclassOf<UBEInventoryItemDefinition> ItemDef) const
{
	int32 TotalCount = 0;
	for (const FBEInventoryEntry& Entry : InventoryList.Entries)
	{
		UBEInventoryItemInstance* Instance = Entry.Instance;

		if (IsValid(Instance))
		{
			if (Instance->GetItemDef() == ItemDef)
			{
				++TotalCount;
			}
		}
	}

	return TotalCount;
}

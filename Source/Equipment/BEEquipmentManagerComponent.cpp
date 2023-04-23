// Copyright Eigi Chin

#include "BEEquipmentManagerComponent.h"

#include "Ability/BEAbilitySystemComponent.h"
#include "Ability/BEAbilitySet.h"
#include "BEEquipmentInstance.h"
#include "BEEquipmentSlotData.h"
#include "Item/BEItemData.h"
#include "Item/BEItemDataFragment_Equippable.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/ActorComponent.h"
#include "Engine/ActorChannel.h"
#include "GameFramework/Actor.h"
#include "Misc/AssertionMacros.h"
#include "Net/UnrealNetwork.h"
#include "Templates/Casts.h"
#include "UObject/Object.h"
#include "UObject/ObjectPtr.h"
#include "UObject/UObjectBaseUtility.h"
#include "GameFramework/GameplayMessageSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEEquipmentManagerComponent)

UE_DEFINE_GAMEPLAY_TAG(TAG_Message_Equipment_SlotChange, "Message.Equipment.SlotChange");
UE_DEFINE_GAMEPLAY_TAG(TAG_Message_Equipment_ActiveSlotChange, "Message.Equipment.ActiveSlotChange");
UE_DEFINE_GAMEPLAY_TAG(TAG_Equipment_Slot, "Equipment.Slot");

class FLifetimeProperty;
struct FReplicationFlags;


//////////////////////////////////////////////////////////////////////
// FBEEquipmentEntry

FString FBEEquipmentEntry::GetDebugString() const
{
	return FString::Printf(TEXT("(Slot: %s) ItemData: %s, Instance: %s"), *SlotTag.GetTagName().ToString(), *GetNameSafe(ItemData), *GetNameSafe(Instance));
}

bool FBEEquipmentEntry::IsValid() const
{
	return (ItemData != nullptr) && (Instance != nullptr) && (SlotTag.IsValid());
}


//////////////////////////////////////////////////////////////////////
// FBEEquipmentList

void FBEEquipmentList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
 	for (int32 Index : RemovedIndices)
 	{
 		const FBEEquipmentEntry& Entry = Entries[Index];
		if (Entry.IsValid())
		{
			Entry.Instance->OnUnequiped();
			BroadcastSlotChangeMessage();

			if (Entry.Activated == true)
			{
				Entry.Instance->OnDeactivated();
			}
		}
 	}
}

void FBEEquipmentList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		const FBEEquipmentEntry& Entry = Entries[Index];
		if (Entry.IsValid())
		{
			Entry.Instance->OnEquiped(Entry.ItemData);
			BroadcastSlotChangeMessage(Entry.SlotTag, Entry.ItemData, Entry.Instance);

			if (Entry.Activated == true)
			{
				Entry.Instance->OnActivated();
				BroadcastActiveSlotChangeMessage(Entry.SlotTag, Entry.ItemData, Entry.Instance);
			}
		}
	}
}

void FBEEquipmentList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		const FBEEquipmentEntry& Entry = Entries[Index];
		if (Entry.IsValid())
		{
			if (Entry.Activated == true)
			{
				Entry.Instance->OnActivated();
				BroadcastActiveSlotChangeMessage(Entry.SlotTag, Entry.ItemData, Entry.Instance);
			}
			else
			{
				Entry.Instance->OnDeactivated();
			}
		}
	}
}

void FBEEquipmentList::BroadcastSlotChangeMessage(FGameplayTag SlotTag, UBEItemData* ItemData, UBEEquipmentInstance* Instance)
{
	FBEEquipmentSlotChangedMessage Message;
	Message.OwnerComponent = OwnerComponent;
	Message.SlotTag = SlotTag;
	Message.ItemData = ItemData;
	Message.Instance = Instance;

	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(OwnerComponent->GetWorld());
	MessageSystem.BroadcastMessage(TAG_Message_Equipment_SlotChange, Message);
}

void FBEEquipmentList::BroadcastActiveSlotChangeMessage(FGameplayTag SlotTag, UBEItemData* ItemData, UBEEquipmentInstance* Instance)
{
	FBEEquipmentSlotChangedMessage Message;
	Message.OwnerComponent = OwnerComponent;
	Message.SlotTag = SlotTag;
	Message.ItemData = ItemData;
	Message.Instance = Instance;

	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(OwnerComponent->GetWorld());
	MessageSystem.BroadcastMessage(TAG_Message_Equipment_ActiveSlotChange, Message);
}


UBEEquipmentInstance* FBEEquipmentList::AddEntry(UBEItemData* ItemData, FGameplayTag SlotTag, UBEAbilitySystemComponent* ASC)
{
	const UBEItemDataFragment_Equippable* Fragment = ItemData->FindFragmentByClass<UBEItemDataFragment_Equippable>();
	if (Fragment == nullptr)
	{
		return nullptr;
	}

	const TSubclassOf<UBEEquipmentInstance> InstanceType = Fragment->InstanceType;
	if (InstanceType == nullptr)
	{
		return nullptr;
	}

	FBEEquipmentEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.SlotTag	= SlotTag;
	NewEntry.ItemData	= ItemData;
	NewEntry.Instance	= NewObject<UBEEquipmentInstance>(OwnerComponent->GetOwner(), InstanceType);
	NewEntry.Instance->OnEquiped(ItemData);

	for (TObjectPtr<const UBEAbilitySet> AbilitySet : Fragment->AbilitySetsToGrantOnEquip)
	{
		AbilitySet->GiveToAbilitySystem(ASC, /*out=*/ &NewEntry.GrantedHandles_Equip, NewEntry.Instance);
	}

	MarkItemDirty(NewEntry);

	return NewEntry.Instance;
}

void FBEEquipmentList::RemoveEntry(int32 Index, UBEAbilitySystemComponent* ASC)
{
	FBEEquipmentEntry& Entry = Entries[Index];
	
	if (Entry.Activated == true)
	{
		Entry.GrantedHandles_Active.TakeFromAbilitySystem(ASC);
		Entry.GrantedHandles_Equip.TakeFromAbilitySystem(ASC);

		if (UBEEquipmentInstance* Instance = Entry.Instance)
		{
			Instance->OnDeactivated();
			Instance->OnUnequiped();
		}
	}
	else
	{
		Entry.GrantedHandles_Equip.TakeFromAbilitySystem(ASC);

		if (UBEEquipmentInstance* Instance = Entry.Instance)
		{
			Instance->OnUnequiped();
		}
	}
}


void FBEEquipmentList::ActivateEntry(int32 Index, UBEAbilitySystemComponent* ASC)
{
	FBEEquipmentEntry& Entry = Entries[Index];

	const UBEItemData* ItemData = Entry.ItemData;
	if (ItemData == nullptr)
	{
		return;
	}

	const UBEItemDataFragment_Equippable* Fragment = ItemData->FindFragmentByClass<UBEItemDataFragment_Equippable>();
	if (Fragment == nullptr)
	{
		return;
	}

	for (TObjectPtr<const UBEAbilitySet> AbilitySet : Fragment->AbilitySetsToGrantOnActive)
	{
		AbilitySet->GiveToAbilitySystem(ASC, /*out=*/ &Entries[Index].GrantedHandles_Active, Entries[Index].Instance);
	}

	if (UBEEquipmentInstance* Instance = Entry.Instance)
	{
		Instance->OnActivated();
	}

	Entry.Activated = true;

	MarkItemDirty(Entry);
}

void FBEEquipmentList::DeactivateEntry(int32 Index, UBEAbilitySystemComponent* ASC)
{
	FBEEquipmentEntry& Entry = Entries[Index];

	Entry.GrantedHandles_Active.TakeFromAbilitySystem(ASC);

	if (UBEEquipmentInstance* Instance = Entry.Instance)
	{
		Instance->OnDeactivated();
	}

	Entry.Activated = false;

	MarkItemDirty(Entry);
}


//////////////////////////////////////////////////////////////////////
// UBEEquipmentManagerComponent

UBEEquipmentManagerComponent::UBEEquipmentManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, EquipmentList(this)
{
	SetIsReplicatedByDefault(true);
	bWantsInitializeComponent = true;
}


void UBEEquipmentManagerComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, EquipmentList);
	DOREPLIFETIME(ThisClass, ActiveSlot);
}

bool UBEEquipmentManagerComponent::ReplicateSubobjects(UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (FBEEquipmentEntry& Entry : EquipmentList.Entries)
	{
		UBEEquipmentInstance* Instance = Entry.Instance;

		if (IsValid(Instance))
		{
			WroteSomething |= Channel->ReplicateSubobject(Instance, *Bunch, *RepFlags);
		}
	}

	return WroteSomething;
}

void UBEEquipmentManagerComponent::UninitializeComponent()
{
	RemoveAllEquipments();

	Super::UninitializeComponent();
}

void UBEEquipmentManagerComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	if (IsUsingRegisteredSubObjectList())
	{
		for (const FBEEquipmentEntry& Entry : EquipmentList.Entries)
		{
			UBEEquipmentInstance* Instance = Entry.Instance;

			if (IsValid(Instance))
			{
				AddReplicatedSubObject(Instance);
			}
		}
	}
}


UBEAbilitySystemComponent* UBEEquipmentManagerComponent::GetAbilitySystemComponent() const
{
	return Cast<UBEAbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner()));
}


bool UBEEquipmentManagerComponent::AddEquipment(FGameplayTag SlotTag, UBEItemData* ItemData, bool ActivateImmediately)
{
	check(GetOwner()->HasAuthority());
	check(SlotTag.IsValid());
	check(ItemData != nullptr);

	UBEAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	check(ASC != nullptr);

	if (AddedSlotTags.Contains(SlotTag))
	{
		return false;
	}

	if (UBEEquipmentInstance* Result = EquipmentList.AddEntry(ItemData, SlotTag, ASC))
	{
		if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
		{
			AddReplicatedSubObject(Result);
		}

		if (ActivateImmediately == true)
		{
			SetActiveSlot(SlotTag);
		}

		AddedSlotTags.Add(SlotTag);

		return true;
	}

	return false;
}

void UBEEquipmentManagerComponent::AddEquipments(const TMap<FGameplayTag, UBEItemData*>& ItemDatas, FGameplayTag ActivateSlotTag)
{
	check(GetOwner()->HasAuthority());

	UBEAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	check(ASC != nullptr);

	for (const auto& KVP : ItemDatas)
	{
		if ((!KVP.Key.IsValid()) || (KVP.Value == nullptr) || (AddedSlotTags.Contains(KVP.Key)))
		{
			continue;
		}

		if (UBEEquipmentInstance* Result = EquipmentList.AddEntry(KVP.Value, KVP.Key, ASC))
		{
			if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
			{
				AddReplicatedSubObject(Result);
			}

			AddedSlotTags.Add(KVP.Key);
		}
	}

	if (ActivateSlotTag.IsValid())
	{
		SetActiveSlot(ActivateSlotTag);
	}
}

bool UBEEquipmentManagerComponent::RemoveEquipment(FGameplayTag SlotTag)
{
	if (!GetOwner()->HasAuthority())
	{
		return false;
	}

	if (!SlotTag.IsValid())
	{
		return false;
	}

	UBEAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (ASC == nullptr)
	{
		return false;
	}

	for (auto EntryIt = EquipmentList.Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FBEEquipmentEntry& Entry = *EntryIt;
		if (Entry.SlotTag == SlotTag)
		{
			if (IsUsingRegisteredSubObjectList())
			{
				RemoveReplicatedSubObject(Entry.Instance);
			}
			
			EquipmentList.RemoveEntry(EntryIt.GetIndex(), ASC);
			EquipmentList.Entries.RemoveAt(Index);
			EquipmentList.MarkArrayDirty();

			AddedSlotTags.Remove(Entry.SlotTag);

			return true;
		}
	}

	return false;
}

void UBEEquipmentManagerComponent::RemoveAllEquipments()
{
	if (!GetOwner()->HasAuthority())
	{
		return false;
	}

	UBEAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (ASC == nullptr)
	{
		return false;
	}

	for (auto EntryIt = EquipmentList.Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FBEEquipmentEntry& Entry = *EntryIt;

		if (IsUsingRegisteredSubObjectList())
		{
			RemoveReplicatedSubObject(Entry.Instance);
		}

		EquipmentList.RemoveEntry(EntryIt.GetIndex(), ASC);

		AddedSlotTags.Remove(Entry.SlotTag);
	}

	EquipmentList.Entries.RemoveAll();
	EquipmentList.MarkArrayDirty();
}


void UBEEquipmentManagerComponent::SetActiveSlot(FGameplayTag SlotTag)
{
	check(GetOwner()->HasAuthority());
	check(SlotTag.IsValid());

	UBEAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	check(ASC != nullptr);

	int32 LastActiveIndex, NewActiveIndex = INDEX_NONE;
	for (auto EntryIt = EquipmentList.Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FBEEquipmentEntry& Entry = *EntryIt;

		if (Entry.Activated == true)
		{
			LastActiveIndex = EntryIt.GetIndex();
		}

		if (Entry.SlotTag == SlotTag)
		{
			NewActiveIndex = EntryIt.GetIndex();
		}
	}

	check(NewActiveIndex != INDEX_NONE);
	check(NewActiveIndex != LastActiveIndex);
	
	if (LastActiveIndex != INDEX_NONE)
	{
		EquipmentList.DeactivateEntry(LastActiveIndex, ASC);
	}

	EquipmentList.ActivateEntry(NewActiveIndex, ASC);
}

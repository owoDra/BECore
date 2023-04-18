// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BEQuickBarComponent.h"

#include "Inventory/BEInventoryItemInstance.h"
#include "Inventory/InventoryFragment_EquippableItem.h"
#include "Equipment/BEEquipmentInstance.h"
#include "Equipment/BEEquipmentDefinition.h"
#include "Equipment/BEEquipmentManagerComponent.h"
#include "BEGameplayTags.h"

#include "CoreTypes.h"
#include "GameFramework/Controller.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/Pawn.h"
#include "Misc/AssertionMacros.h"
#include "Net/UnrealNetwork.h"
#include "Templates/Casts.h"
#include "Templates/SubclassOf.h"
#include "UObject/NameTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEQuickBarComponent)

class FLifetimeProperty;
class ULyraEquipmentDefinition;


// =====================================
//   初期化
// =====================================

UBEQuickBarComponent::UBEQuickBarComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

void UBEQuickBarComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Slots);
	DOREPLIFETIME(ThisClass, ActiveSlot);
}

void UBEQuickBarComponent::BeginPlay()
{
	if (Slots.Num() < NumSlots)
	{
		Slots.AddDefaulted(NumSlots - Slots.Num());
	}

	Super::BeginPlay();
}


// =====================================
//  ユーティリティ
// =====================================

UBEEquipmentManagerComponent* UBEQuickBarComponent::FindEquipmentManager() const
{
	if (AController* OwnerController = Cast<AController>(GetOwner()))
	{
		if (APawn* Pawn = OwnerController->GetPawn())
		{
			return Pawn->FindComponentByClass<UBEEquipmentManagerComponent>();
		}
	}
	return nullptr;
}

int32 UBEQuickBarComponent::GetNextFreeItemSlot() const
{
	int32 SlotIndex = 0;

	for (TObjectPtr<UBEInventoryItemInstance> ItemPtr : Slots)
	{
		if (ItemPtr == nullptr)
		{
			return SlotIndex;
		}
		++SlotIndex;
	}

	return INDEX_NONE;
}

void UBEQuickBarComponent::CycleActiveSlotForward()
{
	if (Slots.Num() < 2)
	{
		return;
	}

	const int32 OldIndex = (ActiveSlot < 0 ? Slots.Num() - 1 : ActiveSlot);
	int32 NewIndex = ActiveSlot;
	do
	{
		NewIndex = (NewIndex + 1) % Slots.Num();
		if (Slots[NewIndex] != nullptr)
		{
			SetActiveSlot(NewIndex);
			return;
		}
	} while (NewIndex != OldIndex);
}

void UBEQuickBarComponent::CycleActiveSlotBackward()
{
	if (Slots.Num() < 2)
	{
		return;
	}

	const int32 OldIndex = (ActiveSlot < 0 ? Slots.Num() - 1 : ActiveSlot);
	int32 NewIndex = ActiveSlot;
	do
	{
		NewIndex = (NewIndex - 1 + Slots.Num()) % Slots.Num();
		if (Slots[NewIndex] != nullptr)
		{
			SetActiveSlot(NewIndex);
			return;
		}
	} while (NewIndex != OldIndex);
}

bool UBEQuickBarComponent::DoseEquipWhenAdded(UBEInventoryItemInstance* Item)
{
	if (Item == nullptr)
	{
		return false;
	}

	const UInventoryFragment_EquippableItem* EquipInfo = Item->FindFragmentByClass<UInventoryFragment_EquippableItem>();
	if (EquipInfo == nullptr)
	{
		return false;
	}

	TSubclassOf<UBEEquipmentDefinition> EquipDef = EquipInfo->EquipmentDefinition;
	if (EquipDef == nullptr)
	{
		return false;
	}

	if (!EquipDef.GetDefaultObject()->EquipWhenAdded)
	{
		return false;
	}

	return true;
}

bool UBEQuickBarComponent::DidEquipedWhenAdded(int32 SlotIndex)
{
	UBEInventoryItemInstance* Item = Slots[SlotIndex];

	if (!DoseEquipWhenAdded(Item))
	{
		return false;
	}

	if (!ActivatedSubItems.Find(SlotIndex))
	{
		return false;
	}
	
	return true;
}


// =====================================
//  スロット
// =====================================

void UBEQuickBarComponent::OnRep_Slots()
{
	FBEQuickBarSlotsChangedMessage Message;
	Message.Owner = GetOwner();
	Message.Slots = Slots;

	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(this);
	MessageSystem.BroadcastMessage(TAG_Message_QuickBar_SlotsChanged, Message);
}

void UBEQuickBarComponent::OnRep_ActiveSlot()
{
	FBEQuickBarActiveIndexChangedMessage Message;
	Message.Owner = GetOwner();
	Message.ActiveIndex = ActiveSlot;

	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(this);
	MessageSystem.BroadcastMessage(TAG_Message_QuickBar_ActiveIndexChanged, Message);
}


void UBEQuickBarComponent::SetActiveSlot_Implementation(int32 NewIndex)
{
	if (Slots.IsValidIndex(NewIndex) && ((ActiveSlot != NewIndex) || (ActiveItem == nullptr)) && (!DidEquipedWhenAdded(NewIndex)) && IsValid(Slots[NewIndex]))
	{
		UnequipItem(ActiveItem);
		ActiveItem = nullptr;

		ActiveSlot = NewIndex;

		ActiveItem = EquipItem(Slots[ActiveSlot]);

		OnRep_ActiveSlot();
	}
}

void UBEQuickBarComponent::AddItemToSlot(int32 SlotIndex, UBEInventoryItemInstance* Item)
{
	if (Slots.IsValidIndex(SlotIndex) && (Item != nullptr))
	{
		if (Slots[SlotIndex] != nullptr)
		{
			RemoveItemFromSlot(SlotIndex);
		}

		Slots[SlotIndex] = Item;

		if (DoseEquipWhenAdded(Item))
		{
			UBEEquipmentInstance* ItemAdded = EquipItem(Item);

			ActivatedSubItems.Add(SlotIndex, ItemAdded);
		}
		
		OnRep_Slots();
	}
}

UBEInventoryItemInstance* UBEQuickBarComponent::RemoveItemFromSlot(int32 SlotIndex)
{
	UBEInventoryItemInstance* Result = nullptr;

	if (ActiveSlot == SlotIndex)
	{
		UnequipItem(ActiveItem);
		ActiveItem = nullptr;
	}

	if (auto* FoundItem = ActivatedSubItems.Find(SlotIndex))
	{
		if (UBEEquipmentInstance* Item = *FoundItem)
		{
			UnequipItem(Item);
			ActivatedSubItems.Remove(SlotIndex);
		}
	}

	if (Slots.IsValidIndex(SlotIndex))
	{
		Result = Slots[SlotIndex];

		if (Result != nullptr)
		{
			Slots[SlotIndex] = nullptr;
			OnRep_Slots();
		}
	}

	return Result;
}


UBEEquipmentInstance* UBEQuickBarComponent::EquipItem(UBEInventoryItemInstance* Item)
{
	if (Item == nullptr)
	{
		return nullptr;
	}

	const UInventoryFragment_EquippableItem* EquipInfo = Item->FindFragmentByClass<UInventoryFragment_EquippableItem>();
	if (EquipInfo == nullptr)
	{
		return nullptr;
	}

	TSubclassOf<UBEEquipmentDefinition> EquipDef = EquipInfo->EquipmentDefinition;
	if (EquipDef == nullptr)
	{
		return nullptr;
	}

	if (!EquipDef.GetDefaultObject()->EquipWhenAdded)
	{
		if (ActiveItem != nullptr)
		{
			return nullptr;
		}
	}

	if (UBEEquipmentManagerComponent* EquipmentManager = FindEquipmentManager())
	{
		UBEEquipmentInstance* ItemEquiped = EquipmentManager->EquipItem(EquipDef);
		if (ItemEquiped != nullptr)
		{
			ItemEquiped->SetInstigator(Item);
		}

		return ItemEquiped;
	}

	return nullptr;
}

void UBEQuickBarComponent::UnequipItem(UBEEquipmentInstance* Item)
{
	if (UBEEquipmentManagerComponent* EquipmentManager = FindEquipmentManager())
	{
		if (Item != nullptr)
		{
			EquipmentManager->UnequipItem(Item);
		}
	}
}


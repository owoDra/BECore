// Copyright Eigi Chin

#include "BEPawnEquipmentComponent.h"

#include "Ability/BEAbilitySystemComponent.h"
#include "Ability/BEAbilitySet.h"
#include "Character/BEPawnData.h"
#include "Player/BEPlayerController.h"
#include "Player/BEPlayerState.h"
#include "Item/Equipment/BEEquipmentInstance.h"
#include "Item/BEItemData.h"
#include "Item/Fragment/BEItemDataFragment_Equippable.h"
#include "GameplayTag/BETags_Message.h"
#include "GameplayTag/BETags_Item.h"
#include "GameplayTag/BETags_InitState.h"

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
#include "Components/GameFrameworkComponentManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEPawnEquipmentComponent)

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

void FBEEquipmentList::BroadcastSlotChangeMessage(FGameplayTag SlotTag, const UBEItemData* ItemData, UBEEquipmentInstance* Instance)
{
	FBEEquipmentSlotChangedMessage Message;
	Message.OwnerComponent = OwnerComponent;
	Message.SlotTag = SlotTag;
	Message.ItemData = ItemData;
	Message.Instance = Instance;

	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(OwnerComponent->GetWorld());
	MessageSystem.BroadcastMessage(TAG_Message_Equipment_SlotChange, Message);
}

void FBEEquipmentList::BroadcastActiveSlotChangeMessage(FGameplayTag SlotTag, const UBEItemData* ItemData, UBEEquipmentInstance* Instance)
{
	FBEEquipmentSlotChangedMessage Message;
	Message.OwnerComponent = OwnerComponent;
	Message.SlotTag = SlotTag;
	Message.ItemData = ItemData;
	Message.Instance = Instance;

	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(OwnerComponent->GetWorld());
	MessageSystem.BroadcastMessage(TAG_Message_Equipment_ActiveSlotChange, Message);
}


UBEEquipmentInstance* FBEEquipmentList::AddEntry(const UBEItemData* ItemData, FGameplayTag SlotTag, UBEAbilitySystemComponent* ASC)
{
	const UBEItemDataFragment_Equippable* Fragment = ItemData->FindFragmentByClass<UBEItemDataFragment_Equippable>();
	if (Fragment == nullptr)
	{
		return nullptr;
	}

	if (!Fragment->AllowedSlotTags.HasTag(SlotTag))
	{
		return nullptr;
	}

	TSubclassOf<UBEEquipmentInstance> InstanceType = Fragment->InstanceType;
	if (InstanceType == nullptr)
	{
		InstanceType = UBEEquipmentInstance::StaticClass();
	}

	FBEEquipmentEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.SlotTag	= SlotTag;
	NewEntry.ItemData	= ItemData;
	NewEntry.Instance	= NewObject<UBEEquipmentInstance>(OwnerComponent->GetOwner(), InstanceType);
	NewEntry.Instance->OnEquiped(ItemData);
	
	for (auto& Stats : Fragment->InitialEquipmentStats)
	{
		NewEntry.Instance->AddStatTagStack(Stats.Key, Stats.Value);
	}

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
		Instance->SpawnEquipmentMeshes(Fragment->MeshesToSpawn);
		Instance->ApplyAnimLayer(Fragment->AnimLayerToApplyToTPP, Fragment->AnimLayerToApplyToFPP);
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
// UBEPawnEquipmentComponent

const FName UBEPawnEquipmentComponent::NAME_ActorFeatureName("CharacterEquipment");


UBEPawnEquipmentComponent::UBEPawnEquipmentComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, EquipmentList(this)
{
	SetIsReplicatedByDefault(true);
	bWantsInitializeComponent = true;
}

void UBEPawnEquipmentComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, EquipmentList);
}


void UBEPawnEquipmentComponent::OnRegister()
{
	Super::OnRegister();

	const APawn* Pawn = GetOwner<APawn>();
	ensureAlwaysMsgf((Pawn != nullptr), TEXT("UBEPawnEquipmentComponent on [%s] can only be added to Pawn actors."), *GetNameSafe(GetOwner()));

	TArray<UActorComponent*> Components;
	Pawn->GetComponents(UBEPawnEquipmentComponent::StaticClass(), Components);
	ensureAlwaysMsgf((Components.Num() == 1), TEXT("Only one UBEPawnEquipmentComponent should exist on [%s]."), *GetNameSafe(GetOwner()));

	// コンポーネントが GameWorld に存在する際に　InitStateSystem に登録する
	RegisterInitStateFeature();
}

void UBEPawnEquipmentComponent::BeginPlay()
{
	// すべての Feature への変更をリッスンする
	BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);

	// InisStateSystem にこのコンポーネントがスポーンしたことを知らせる。
	ensure(TryToChangeInitState(TAG_InitState_Spawned));

	// 残りの初期化を行う
	CheckDefaultInitialization();
}

void UBEPawnEquipmentComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
}


bool UBEPawnEquipmentComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);

	APawn* Pawn = GetOwner<APawn>();

	/**
	 * [InitState None] -> [InitState Spawned]
	 *
	 *  Pawn が有効
	 */
	if (!CurrentState.IsValid() && DesiredState == TAG_InitState_Spawned)
	{
		if (Pawn != nullptr)
		{
			return true;
		}
	}

	/**
	 * [InitState Spawned] -> [InitState DataAvailable]
	 *
	 *  無条件で許可
	 */
	else if (CurrentState == TAG_InitState_Spawned && DesiredState == TAG_InitState_DataAvailable)
	{
		return true;
	}

	/**
	 * [InitState DataAvailable] -> [InitState DataInitialized]
	 *
	 *  他のすべての Feature が DataAvailable に到達している
	 */
	else if (CurrentState == TAG_InitState_DataAvailable && DesiredState == TAG_InitState_DataInitialized)
	{
		// CharacterBasicComponent が DataInitialized に到達しているか
		// つまり、他のすべての Feature が DataAvailable に到達しているか
		return Manager->HasFeatureReachedInitState(Pawn, UBEPawnBasicComponent::NAME_ActorFeatureName, TAG_InitState_DataInitialized);
	}

	/**
	 * [InitState DataInitialized] -> [InitState GameplayReady]
	 *
	 *  無条件で許可
	 */
	else if (CurrentState == TAG_InitState_DataInitialized && DesiredState == TAG_InitState_GameplayReady)
	{
		return true;
	}

	return false;
}

void UBEPawnEquipmentComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	/**
	 * [InitState DataAvailable] -> [InitState DataInitialized]
	 *
	 *  デフォルトの Equipments を適応
	 */
	if (CurrentState == TAG_InitState_DataAvailable && DesiredState == TAG_InitState_DataInitialized)
	{
		APawn* Pawn = GetPawn<APawn>();
		if (!ensure(Pawn))
		{
			return;
		}

		if (!Pawn->HasAuthority())
		{
			return;
		}

		if (const UBEPawnBasicComponent* CharacterBasic = UBEPawnBasicComponent::FindPawnBasicComponent(Pawn))
		{
			if (const UBEPawnData* PawnData = CharacterBasic->GetPawnData())
			{
				if (const UBEEquipmentSet* EquipmentSet= PawnData->EquipmentSet)
				{
					AddEquipments(EquipmentSet->Entries, EquipmentSet->DefaultActiveSlotTag);
				}
			}
		}
	}
}

void UBEPawnEquipmentComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	// CharacterBasicComponent が DataInitialized に到達しているか
	// つまり、他のすべての Feature が DataAvailable に到達しているか
	if (Params.FeatureName == UBEPawnBasicComponent::NAME_ActorFeatureName)
	{
		if (Params.FeatureState == TAG_InitState_DataInitialized)
		{
			CheckDefaultInitialization();
		}
	}
}

void UBEPawnEquipmentComponent::CheckDefaultInitialization()
{
	static const TArray<FGameplayTag> StateChain = {
		TAG_InitState_Spawned,
		TAG_InitState_DataAvailable,
		TAG_InitState_DataInitialized,
		TAG_InitState_GameplayReady
	};

	ContinueInitStateChain(StateChain);
}


bool UBEPawnEquipmentComponent::ReplicateSubobjects(UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
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

void UBEPawnEquipmentComponent::UninitializeComponent()
{
	RemoveAllEquipments();

	Super::UninitializeComponent();
}

void UBEPawnEquipmentComponent::ReadyForReplication()
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


UBEAbilitySystemComponent* UBEPawnEquipmentComponent::GetAbilitySystemComponent() const
{
	return Cast<UBEAbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner()));
}


bool UBEPawnEquipmentComponent::AddEquipment(FGameplayTag SlotTag, const UBEItemData* ItemData, bool ActivateImmediately)
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

void UBEPawnEquipmentComponent::AddEquipments(const TArray<FBEEquipmentSetEntry>& Entries, FGameplayTag ActivateSlotTag)
{
	check(GetOwner()->HasAuthority());

	UBEAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	check(ASC != nullptr);

	for (FBEEquipmentSetEntry Entry : Entries)
	{
		if ((!Entry.SlotTag.IsValid()) || (Entry.ItemData == nullptr) || (AddedSlotTags.Contains(Entry.SlotTag)))
		{
			continue;
		}

		if (UBEEquipmentInstance* Result = EquipmentList.AddEntry(Entry.ItemData, Entry.SlotTag, ASC))
		{
			if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
			{
				AddReplicatedSubObject(Result);
			}

			AddedSlotTags.Add(Entry.SlotTag);
		}
	}

	if (ActivateSlotTag.IsValid())
	{
		SetActiveSlot(ActivateSlotTag);
	}
}

bool UBEPawnEquipmentComponent::RemoveEquipment(FGameplayTag SlotTag)
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
			EquipmentList.Entries.RemoveAt(EntryIt.GetIndex());
			EquipmentList.MarkArrayDirty();

			AddedSlotTags.Remove(Entry.SlotTag);

			return true;
		}
	}

	return false;
}

void UBEPawnEquipmentComponent::RemoveAllEquipments()
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	UBEAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (ASC == nullptr)
	{
		return;
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

	EquipmentList.Entries.Empty();
	EquipmentList.MarkArrayDirty();
}


void UBEPawnEquipmentComponent::SetActiveSlot(FGameplayTag SlotTag)
{
	check(GetOwner()->HasAuthority());
	check(SlotTag.IsValid());

	UBEAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	check(ASC != nullptr);

	int32 LastActiveIndex = INDEX_NONE; 
	int32 NewActiveIndex = INDEX_NONE;
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

bool UBEPawnEquipmentComponent::GetActiveSlotInfo(FBEEquipmentSlotChangedMessage& SlotInfo)
{
	SlotInfo = FBEEquipmentSlotChangedMessage();

	for (FBEEquipmentEntry Entry : EquipmentList.Entries)
	{
		if (Entry.Activated == true)
		{
			SlotInfo.OwnerComponent = this;
			SlotInfo.SlotTag = Entry.SlotTag;
			SlotInfo.Instance = Entry.Instance;
			SlotInfo.ItemData = Entry.ItemData;
			return true;
		}
	}

	return false;
}

bool UBEPawnEquipmentComponent::GetSlotInfo(FGameplayTag SlotTag, FBEEquipmentSlotChangedMessage& SlotInfo)
{
	SlotInfo = FBEEquipmentSlotChangedMessage();

	for (FBEEquipmentEntry Entry : EquipmentList.Entries)
	{
		if (Entry.SlotTag == SlotTag)
		{
			SlotInfo.OwnerComponent = this;
			SlotInfo.SlotTag = Entry.SlotTag;
			SlotInfo.Instance = Entry.Instance;
			SlotInfo.ItemData = Entry.ItemData;
			return true;
		}
	}

	return false;
}

UBEPawnEquipmentComponent* UBEPawnEquipmentComponent::FindPawnEquipmentComponent(const APawn* Pawn)
{
	return (Pawn ? Pawn->FindComponentByClass<UBEPawnEquipmentComponent>() : nullptr);
}

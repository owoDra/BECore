// Copyright Eigi Chin

#include "BECharacterEquipmentComponent.h"

#include "Ability/BEAbilitySystemComponent.h"
#include "Ability/BEAbilitySet.h"
#include "Character/BECharacterData.h"
#include "Player/BEPlayerController.h"
#include "Player/BEPlayerState.h"
#include "Equipment/BEEquipmentInstance.h"
#include "Item/BEItemData.h"
#include "Item/Fragment/BEItemDataFragment_Equippable.h"
#include "BEGameplayTags.h"

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

#include UE_INLINE_GENERATED_CPP_BY_NAME(BECharacterEquipmentComponent)

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
// UBECharacterEquipmentComponent

const FName UBECharacterEquipmentComponent::NAME_ActorFeatureName("CharacterEquipment");


UBECharacterEquipmentComponent::UBECharacterEquipmentComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, EquipmentList(this)
{
	SetIsReplicatedByDefault(true);
	bWantsInitializeComponent = true;
}

void UBECharacterEquipmentComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, EquipmentList);
}


void UBECharacterEquipmentComponent::OnRegister()
{
	Super::OnRegister();

	const APawn* Pawn = GetOwner<APawn>();
	ensureAlwaysMsgf((Pawn != nullptr), TEXT("UBECharacterEquipmentComponent on [%s] can only be added to Pawn actors."), *GetNameSafe(GetOwner()));

	TArray<UActorComponent*> Components;
	Pawn->GetComponents(UBECharacterEquipmentComponent::StaticClass(), Components);
	ensureAlwaysMsgf((Components.Num() == 1), TEXT("Only one UBECharacterEquipmentComponent should exist on [%s]."), *GetNameSafe(GetOwner()));

	// コンポーネントが GameWorld に存在する際に　InitStateSystem に登録する
	RegisterInitStateFeature();
}

void UBECharacterEquipmentComponent::BeginPlay()
{
	// すべての Feature への変更をリッスンする
	BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);

	// InisStateSystem にこのコンポーネントがスポーンしたことを知らせる。
	ensure(TryToChangeInitState(TAG_InitState_Spawned));

	// 残りの初期化を行う
	CheckDefaultInitialization();
}

void UBECharacterEquipmentComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
}


bool UBECharacterEquipmentComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
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
	 *  Controller が有効
	 */
	else if (CurrentState == TAG_InitState_Spawned && DesiredState == TAG_InitState_DataAvailable)
	{
		// シミュレーションでなく、かつ Controller が有効
		if (Pawn->GetLocalRole() != ROLE_SimulatedProxy)
		{
			AController* Controller = Pawn->GetController();
			if (Controller == nullptr)
			{
				return false;
			}
		}

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
		return Manager->HasFeatureReachedInitState(Pawn, UBECharacterBasicComponent::NAME_ActorFeatureName, TAG_InitState_DataInitialized);
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

void UBECharacterEquipmentComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
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

		if (const UBECharacterBasicComponent* CharacterBasic = UBECharacterBasicComponent::FindCharacterBasicComponent(Pawn))
		{
			if (const UBECharacterData* CharacterData = CharacterBasic->GetCharacterData())
			{
				if (!CharacterData->DefaultEquipments.IsEmpty())
				{
					AddEquipments(CharacterData->DefaultEquipments, CharacterData->DefaultActiveSlotTag);
				}
			}
		}
	}
}

void UBECharacterEquipmentComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	// CharacterBasicComponent が DataInitialized に到達しているか
	// つまり、他のすべての Feature が DataAvailable に到達しているか
	if (Params.FeatureName == UBECharacterBasicComponent::NAME_ActorFeatureName)
	{
		if (Params.FeatureState == TAG_InitState_DataInitialized)
		{
			CheckDefaultInitialization();
		}
	}
}

void UBECharacterEquipmentComponent::CheckDefaultInitialization()
{
	static const TArray<FGameplayTag> StateChain = {
		TAG_InitState_Spawned,
		TAG_InitState_DataAvailable,
		TAG_InitState_DataInitialized,
		TAG_InitState_GameplayReady
	};

	ContinueInitStateChain(StateChain);
}


bool UBECharacterEquipmentComponent::ReplicateSubobjects(UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
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

void UBECharacterEquipmentComponent::UninitializeComponent()
{
	RemoveAllEquipments();

	Super::UninitializeComponent();
}

void UBECharacterEquipmentComponent::ReadyForReplication()
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


UBEAbilitySystemComponent* UBECharacterEquipmentComponent::GetAbilitySystemComponent() const
{
	return Cast<UBEAbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner()));
}


bool UBECharacterEquipmentComponent::AddEquipment(FGameplayTag SlotTag, const UBEItemData* ItemData, bool ActivateImmediately)
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

void UBECharacterEquipmentComponent::AddEquipments(const TMap<FGameplayTag, const UBEItemData*>& ItemDatas, FGameplayTag ActivateSlotTag)
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

bool UBECharacterEquipmentComponent::RemoveEquipment(FGameplayTag SlotTag)
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

void UBECharacterEquipmentComponent::RemoveAllEquipments()
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


void UBECharacterEquipmentComponent::SetActiveSlot(FGameplayTag SlotTag)
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

bool UBECharacterEquipmentComponent::GetActiveSlotItem(FGameplayTag& SlotTag, const UBEItemData*& ItemData, UBEEquipmentInstance*& Instance)
{
	SlotTag  = FGameplayTag::EmptyTag;
	ItemData = nullptr;
	Instance = nullptr;

	for (FBEEquipmentEntry Entry : EquipmentList.Entries)
	{
		if (Entry.Activated == true)
		{
			SlotTag  = Entry.SlotTag;
			ItemData = Entry.ItemData;
			Instance = Entry.Instance;
			return true;
		}
	}

	return false;
}

bool UBECharacterEquipmentComponent::GetSlotItem(FGameplayTag SlotTag, const UBEItemData*& ItemData, UBEEquipmentInstance*& Instance)
{
	ItemData = nullptr;
	Instance = nullptr;

	for (FBEEquipmentEntry Entry : EquipmentList.Entries)
	{
		if (Entry.SlotTag == SlotTag)
		{
			ItemData = Entry.ItemData;
			Instance = Entry.Instance;
			return true;
		}
	}

	return false;
}

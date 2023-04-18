// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright RutenStudio.

#include "BEInventoryItemInstance.h"

#include "BEInventoryItemDefinition.h"
#include "Fragment/BEInventoryItemFragment.h"

#include "Containers/Array.h"
#include "GameplayTagContainer.h"
#include "Misc/AssertionMacros.h"
#include "Net/UnrealNetwork.h"
#include "UObject/Class.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif // UE_WITH_IRIS

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEInventoryItemInstance)

class FLifetimeProperty;


UBEInventoryItemInstance::UBEInventoryItemInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UBEInventoryItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, StatTags);
	DOREPLIFETIME(ThisClass, ItemDef);
}

void UBEInventoryItemInstance::AddStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTags.AddStack(Tag, StackCount);
}

void UBEInventoryItemInstance::RemoveStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTags.RemoveStack(Tag, StackCount);
}

int32 UBEInventoryItemInstance::GetStatTagStackCount(FGameplayTag Tag) const
{
	return StatTags.GetStackCount(Tag);
}

bool UBEInventoryItemInstance::HasStatTag(FGameplayTag Tag) const
{
	return StatTags.ContainsTag(Tag);
}

void UBEInventoryItemInstance::SetItemDef(TSubclassOf<UBEInventoryItemDefinition> InDef)
{
	ItemDef = InDef;
}

const UBEInventoryItemFragment* UBEInventoryItemInstance::FindFragmentByClass(TSubclassOf<UBEInventoryItemFragment> FragmentClass) const
{
	if ((ItemDef != nullptr) && (FragmentClass != nullptr))
	{
		return GetDefault<UBEInventoryItemDefinition>(ItemDef)->FindFragmentByClass(FragmentClass);
	}

	return nullptr;
}

#if UE_WITH_IRIS
void UBEInventoryItemInstance::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	using namespace UE::Net;

	Super::RegisterReplicationFragments(Context, RegistrationFlags);

	// Build descriptors and allocate PropertyReplicationFragments for this object
	FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}
#endif // UE_WITH_IRIS

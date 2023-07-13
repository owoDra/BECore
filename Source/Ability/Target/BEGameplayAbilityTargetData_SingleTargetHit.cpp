// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#include "BEGameplayAbilityTargetData_SingleTargetHit.h"

#include "Ability/BEGameplayEffectContext.h"
#include "Serialization/Archive.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEGameplayAbilityTargetData_SingleTargetHit)


//////////////////////////////////////////////////////////////////////

void FBEGameplayAbilityTargetData_SingleTargetHit::AddTargetDataToContext(FGameplayEffectContextHandle& Context, bool bIncludeActorArray) const
{
	FGameplayAbilityTargetData_SingleTargetHit::AddTargetDataToContext(Context, bIncludeActorArray);

	// Add game-specific data
	if (FBEGameplayEffectContext* TypedContext = FBEGameplayEffectContext::ExtractEffectContext(Context))
	{
		TypedContext->CartridgeID = CartridgeID;
	}
}

bool FBEGameplayAbilityTargetData_SingleTargetHit::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	FGameplayAbilityTargetData_SingleTargetHit::NetSerialize(Ar, Map, bOutSuccess);

	Ar << CartridgeID;

	return true;
}

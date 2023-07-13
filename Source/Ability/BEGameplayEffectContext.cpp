// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#include "Ability/BEGameplayEffectContext.h"

#include "Ability/BEAbilitySourceInterface.h"
#include "Engine/HitResult.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Templates/Casts.h"
#include "UObject/Object.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationState/PropertyNetSerializerInfoRegistry.h"
#include "Serialization/GameplayEffectContextNetSerializer.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEGameplayEffectContext)


FBEGameplayEffectContext* FBEGameplayEffectContext::ExtractEffectContext(struct FGameplayEffectContextHandle Handle)
{
	FGameplayEffectContext* BaseEffectContext = Handle.Get();
	if ((BaseEffectContext != nullptr) && BaseEffectContext->GetScriptStruct()->IsChildOf(FBEGameplayEffectContext::StaticStruct()))
	{
		return (FBEGameplayEffectContext*)BaseEffectContext;
	}

	return nullptr;
}

bool FBEGameplayEffectContext::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	FGameplayEffectContext::NetSerialize(Ar, Map, bOutSuccess);

	// Not serialized for post-activation use:
	// CartridgeID

	return true;
}

#if UE_WITH_IRIS
namespace UE::Net
{
	// Forward to FGameplayEffectContextNetSerializer
	// Note: If FBEGameplayEffectContext::NetSerialize() is modified, a custom NetSerializesr must be implemented as the current fallback will no longer be sufficient.
	UE_NET_IMPLEMENT_FORWARDING_NETSERIALIZER_AND_REGISTRY_DELEGATES(BEGameplayEffectContext, FGameplayEffectContextNetSerializer);
}
#endif

void FBEGameplayEffectContext::SetAbilitySource(const IBEAbilitySourceInterface* InObject, float InSourceLevel)
{
	AbilitySourceObject = MakeWeakObjectPtr(Cast<const UObject>(InObject));
	//SourceLevel = InSourceLevel;
}

const IBEAbilitySourceInterface* FBEGameplayEffectContext::GetAbilitySource() const
{
	return Cast<IBEAbilitySourceInterface>(AbilitySourceObject.Get());
}

const UPhysicalMaterial* FBEGameplayEffectContext::GetPhysicalMaterial() const
{
	if (const FHitResult* HitResultPtr = GetHitResult())
	{
		return HitResultPtr->PhysMaterial.Get();
	}
	return nullptr;
}

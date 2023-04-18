// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BEAbilitySystemGlobals.h"

#include "BEGameplayEffectContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEAbilitySystemGlobals)


UBEAbilitySystemGlobals::UBEAbilitySystemGlobals(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FGameplayEffectContext* UBEAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FBEGameplayEffectContext();
}

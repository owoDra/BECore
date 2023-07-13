// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#include "BEAttributeSet.h"

#include "Ability/BEAbilitySystemComponent.h"

#include "Misc/AssertionMacros.h"
#include "Templates/Casts.h"
#include "UObject/Object.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEAttributeSet)


UBEAttributeSet::UBEAttributeSet()
{
}

UWorld* UBEAttributeSet::GetWorld() const
{
	const UObject* Outer = GetOuter();
	check(Outer);

	return Outer->GetWorld();
}

UBEAbilitySystemComponent* UBEAttributeSet::GetBEAbilitySystemComponent() const
{
	return Cast<UBEAbilitySystemComponent>(GetOwningAbilitySystemComponent());
}

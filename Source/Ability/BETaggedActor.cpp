// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BETaggedActor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BETaggedActor)


ABETaggedActor::ABETaggedActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ABETaggedActor::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer.AppendTags(StaticGameplayTags);
}

#if WITH_EDITOR
bool ABETaggedActor::CanEditChange(const FProperty* InProperty) const
{
	// Prevent editing of the other tags property
	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AActor, Tags))
	{
		return false;
	}

	return Super::CanEditChange(InProperty);
}
#endif

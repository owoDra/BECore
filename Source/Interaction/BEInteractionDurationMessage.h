// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "BEInteractionDurationMessage.generated.h"


USTRUCT(BlueprintType)
struct FBEInteractionDurationMessage
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> Instigator = nullptr;
	
	UPROPERTY(BlueprintReadWrite)
	float Duration = 0;
};

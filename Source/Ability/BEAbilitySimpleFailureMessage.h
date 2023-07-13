// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "BEAbilitySimpleFailureMessage.generated.h"


USTRUCT(BlueprintType)
struct FBEAbilitySimpleFailureMessage
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
		TObjectPtr<APlayerController> PlayerController = nullptr;

	UPROPERTY(BlueprintReadWrite)
		FGameplayTagContainer FailureTags;

	UPROPERTY(BlueprintReadWrite)
		FText UserFacingReason;
};

USTRUCT(BlueprintType)
struct FBEAbilityMontageFailureMessage
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite)
		APlayerController* PlayerController = nullptr;

	// All the reasons why this ability has failed
	UPROPERTY(BlueprintReadWrite)
		FGameplayTagContainer FailureTags;

	UPROPERTY(BlueprintReadWrite)
		UAnimMontage* FailureMontage = nullptr;
};

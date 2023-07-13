// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "BEVerbMessage.h"

#include "GameplayEffectTypes.h"
#include "UObject/UObjectGlobals.h"

#include "BEVerbMessageHelpers.generated.h"

class APlayerController;
class APlayerState;
class UObject;
struct FFrame;


UCLASS()
class BECORE_API UBEVerbMessageHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "BE")
	static APlayerState* GetPlayerStateFromObject(UObject* Object);

	UFUNCTION(BlueprintCallable, Category = "BE")
	static APlayerController* GetPlayerControllerFromObject(UObject* Object);

	UFUNCTION(BlueprintCallable, Category = "BE")
	static FGameplayCueParameters VerbMessageToCueParameters(const FBEVerbMessage& Message);

	UFUNCTION(BlueprintCallable, Category = "BE")
	static FBEVerbMessage CueParametersToVerbMessage(const FGameplayCueParameters& Params);
};

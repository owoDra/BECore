// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "HAL/Platform.h"
#include "Math/Color.h"
#include "UObject/NameTypes.h"
#include "UObject/UObjectGlobals.h"

#include "BETeamStatics.generated.h"

class UBETeamDisplayAsset;
class UObject;
class UTexture;
struct FFrame;


/** A subsystem for easy access to team information for team-based actors (e.g., pawns or player states) */
UCLASS()
class UBETeamStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	// Returns the team this object belongs to, or INDEX_NONE if it is not part of a team
	UFUNCTION(BlueprintCallable, Category=Teams, meta=(Keywords="GetTeamFromObject", DefaultToSelf="Agent", AdvancedDisplay="bLogIfNotSet"))
	static void FindTeamFromObject(const UObject* Agent, bool& bIsPartOfTeam, int32& TeamId, UBETeamDisplayAsset*& DisplayAsset, bool bLogIfNotSet = false);

	// Returns the enemy team
	UFUNCTION(BlueprintCallable, Category = Teams, meta = (DefaultToSelf = "Agent"))
	static TArray<int32> GetEnemyTeamIDs(const UObject* Agent);

	UFUNCTION(BlueprintCallable, Category=Teams, meta=(WorldContext="WorldContextObject"))
	static UBETeamDisplayAsset* GetTeamDisplayAsset(const UObject* WorldContextObject, int32 TeamId);

	UFUNCTION(BlueprintCallable, Category = Teams)
	static float GetTeamScalarWithFallback(UBETeamDisplayAsset* DisplayAsset, FName ParameterName, float DefaultValue);

	UFUNCTION(BlueprintCallable, Category = Teams)
	static FLinearColor GetTeamColorWithFallback(UBETeamDisplayAsset* DisplayAsset, FName ParameterName, FLinearColor DefaultValue);

	UFUNCTION(BlueprintCallable, Category = Teams)
	static UTexture* GetTeamTextureWithFallback(UBETeamDisplayAsset* DisplayAsset, FName ParameterName, UTexture* DefaultValue);
};

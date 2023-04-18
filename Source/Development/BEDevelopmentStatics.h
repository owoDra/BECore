// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "Templates/SubclassOf.h"
#include "UObject/UObjectGlobals.h"

#include "BEDevelopmentStatics.generated.h"

class UClass;
class UObject;
class UWorld;
struct FAssetData;
struct FFrame;


UCLASS()
class UBEDevelopmentStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Should game logic skip directly to gameplay (skipping any match warmup / waiting for players / etc... aspects)
	// Will always return false except when playing in the editor and bTestFullGameFlowInPIE (in BE Developer Settings) is false
	UFUNCTION(BlueprintCallable, Category = "BE")
		static bool ShouldSkipDirectlyToGameplay();

	// Should game logic load cosmetic backgrounds in the editor?
	// Will always return true except when playing in the editor and bSkipLoadingCosmeticBackgroundsInPIE (in BE Developer Settings) is true
	UFUNCTION(BlueprintCallable, Category = "BE", meta = (ExpandBoolAsExecs = "ReturnValue"))
		static bool ShouldLoadCosmeticBackgrounds();

	/// Should game logic load cosmetic backgrounds in the editor?
	/// Will always return true except when playing in the editor and bSkipLoadingCosmeticBackgroundsInPIE (in BE Developer Settings) is true
	UFUNCTION(BlueprintCallable, Category = "BE")
		static bool CanPlayerBotsAttack();

	// Finds the most appropriate play-in-editor world to run 'server' cheats on
	//   This might be the only world if running standalone, the listen server, or the dedicated server
	static UWorld* FindPlayInEditorAuthorityWorld();

	// Tries to find a class by a short name (with some heuristics to improve the usability when done via a cheat console)
	static UClass* FindClassByShortName(const FString& SearchToken, UClass* DesiredBaseClass, bool bLogFailures = true);

	template <typename DesiredClass>
	static TSubclassOf<DesiredClass> FindClassByShortName(const FString& SearchToken, bool bLogFailures = true)
	{
		return FindClassByShortName(SearchToken, DesiredClass::StaticClass(), bLogFailures);
	}

private:
	static TArray<FAssetData> GetAllBlueprints();
	static UClass* FindBlueprintClass(const FString& TargetNameRaw, UClass* DesiredBaseClass);
};

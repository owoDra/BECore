// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "BEExperienceManager.generated.h"


/**
 * Manager for experiences - primarily for arbitration between multiple PIE sessions
 */
UCLASS(MinimalAPI)
class UBEExperienceManager : public UEngineSubsystem
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	BECORE_API void OnPlayInEditorBegun();

	static void NotifyOfPluginActivation(const FString PluginURL);
	static bool RequestToDeactivatePlugin(const FString PluginURL);
#else
	static void NotifyOfPluginActivation(const FString PluginURL) {}
	static bool RequestToDeactivatePlugin(const FString PluginURL) { return true; }
#endif

private:
	// The map of requests to active count for a given game feature plugin
	// (to allow first in, last out activation management during PIE)
	TMap<FString, int32> GameFeaturePluginRequestCountMap;
};

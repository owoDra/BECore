// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "GameplayTagContainer.h"
#include "BEDeveloperSettings.generated.h"

class UBEExperienceDefinition;


UENUM()
enum class ECheatExecutionTime
{
	// When the cheat manager is created
	OnCheatManagerCreated,

	// When a pawn is possessed by a player
	OnPlayerPawnPossession
};

USTRUCT()
struct FBECheatToRun
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
		ECheatExecutionTime Phase = ECheatExecutionTime::OnPlayerPawnPossession;

	UPROPERTY(EditAnywhere)
		FString Cheat;
};


/**
 * Developer settings / editor cheats
 */
UCLASS(config = EditorPerProjectUserSettings, MinimalAPI)
class UBEDeveloperSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:
	UBEDeveloperSettings();

	//~UDeveloperSettings interface
	virtual FName GetCategoryName() const override;
	//~End of UDeveloperSettings interface

public:
	// The experience override to use for Play in Editor (if not set, the default for the world settings of the open map will be used)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, config, Category = Lyra, meta = (AllowedTypes = "BEExperienceDefinition"))
		FPrimaryAssetId ExperienceOverride;

	UPROPERTY(BlueprintReadOnly, config, Category = "BE|Bots")
		bool bOverrideBotCount = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, config, Category = "BE|Bots", meta = (EditCondition = bOverrideBotCount))
		int32 OverrideNumPlayerBotsToSpawn = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, config, Category = "BE|Bots")
		bool bAllowPlayerBotsToAttack = true;

	// Do the full game flow when playing in the editor, or skip 'waiting for player' / etc... game phases?
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, config, Category = "BE")
		bool bTestFullGameFlowInPIE = false;

	/**
	* Should force feedback effects be played, even if the last input device was not a gamepad?
	* The default behavior in Lyra is to only play force feedback if the most recent input device was a gamepad.
	*/
	UPROPERTY(config, EditAnywhere, Category = "BE", meta = (ConsoleVariable = "BEPC.ShouldAlwaysPlayForceFeedback"))
		bool bShouldAlwaysPlayForceFeedback = false;

	// Should game logic load cosmetic backgrounds in the editor or skip them for iteration speed?
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, config, Category = "BE")
		bool bSkipLoadingCosmeticBackgroundsInPIE = false;

	// List of cheats to auto-run during 'play in editor'
	UPROPERTY(config, EditAnywhere, Category = "BE")
		TArray<FBECheatToRun> CheatsToRun;

	// Should messages broadcast through the gameplay message subsystem be logged?
	UPROPERTY(config, EditAnywhere, Category = GameplayMessages, meta = (ConsoleVariable = "GameplayMessageSubsystem.LogMessages"))
		bool LogGameplayMessages = false;

#if WITH_EDITORONLY_DATA
	/** A list of common maps that will be accessible via the editor detoolbar */
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = Maps, meta = (AllowedClasses = "/Script/Engine.World"))
		TArray<FSoftObjectPath> CommonEditorMaps;
#endif

#if WITH_EDITOR
public:
	// Called by the editor engine to let us pop reminder notifications when cheats are active
	BECORE_API void OnPlayInEditorStarted() const;

private:
	void ApplySettings();
#endif

public:
	//~UObject interface
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostReloadConfig(FProperty* PropertyThatWasLoaded) override;
	virtual void PostInitProperties() override;
#endif
	//~End of UObject interface
};

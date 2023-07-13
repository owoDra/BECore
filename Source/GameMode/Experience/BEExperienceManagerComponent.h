// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#pragma once

#include "CoreMinimal.h"
#include "Components/GameStateComponent.h"
#include "GameFeaturePluginOperationResult.h"
#include "LoadingProcessInterface.h"

#include "BEExperienceManagerComponent.generated.h"

class UBEExperienceDefinition;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnBEExperienceLoaded, const UBEExperienceDefinition* /*Experience*/);

////////////////////////////////////////////////////////////////////

enum class EBEExperienceLoadState
{
	Unloaded,
	Loading,
	LoadingGameFeatures,
	LoadingChaosTestingDelay,
	ExecutingActions,
	Loaded,
	Deactivating
};

////////////////////////////////////////////////////////////////////

UCLASS()
class UBEExperienceManagerComponent final : public UGameStateComponent, public ILoadingProcessInterface
{
	GENERATED_BODY()

public:

	UBEExperienceManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UActorComponent interface
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of UActorComponent interface

	//~ILoadingProcessInterface interface
	virtual bool ShouldShowLoadingScreen(FString& OutReason) const override;
	//~End of ILoadingProcessInterface

#if WITH_SERVER_CODE
	void ServerSetCurrentExperience(FPrimaryAssetId ExperienceId);
#endif

	// Ensures the delegate is called once the experience has been loaded,
	// before others are called.
	// However, if the experience has already loaded, calls the delegate immediately.
	void CallOrRegister_OnExperienceLoaded_HighPriority(FOnBEExperienceLoaded::FDelegate&& Delegate);

	// Ensures the delegate is called once the experience has been loaded
	// If the experience has already loaded, calls the delegate immediately
	void CallOrRegister_OnExperienceLoaded(FOnBEExperienceLoaded::FDelegate&& Delegate);

	// Ensures the delegate is called once the experience has been loaded
	// If the experience has already loaded, calls the delegate immediately
	void CallOrRegister_OnExperienceLoaded_LowPriority(FOnBEExperienceLoaded::FDelegate&& Delegate);

	// This returns the current experience if it is fully loaded, asserting otherwise
	// (i.e., if you called it too soon)
	const UBEExperienceDefinition* GetCurrentExperienceChecked() const;

	// Returns true if the experience is fully loaded
	bool IsExperienceLoaded() const;

private:
	UFUNCTION()
		void OnRep_CurrentExperience();

	void StartExperienceLoad();
	void OnExperienceLoadComplete();
	void OnGameFeaturePluginLoadComplete(const UE::GameFeatures::FResult& Result);
	void OnExperienceFullLoadCompleted();

	void OnActionDeactivationCompleted();
	void OnAllActionsDeactivated();

private:
	UPROPERTY(ReplicatedUsing = OnRep_CurrentExperience)
		TObjectPtr<const UBEExperienceDefinition> CurrentExperience;

	EBEExperienceLoadState LoadState = EBEExperienceLoadState::Unloaded;

	int32 NumGameFeaturePluginsLoading = 0;
	TArray<FString> GameFeaturePluginURLs;

	int32 NumObservedPausers = 0;
	int32 NumExpectedPausers = 0;

	/**
	 * Delegate called when the experience has finished loading just before others
	 * (e.g., subsystems that set up for regular gameplay)
	 */
	FOnBEExperienceLoaded OnExperienceLoaded_HighPriority;

	/** Delegate called when the experience has finished loading */
	FOnBEExperienceLoaded OnExperienceLoaded;

	/** Delegate called when the experience has finished loading */
	FOnBEExperienceLoaded OnExperienceLoaded_LowPriority;
};

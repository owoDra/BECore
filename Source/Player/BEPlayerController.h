// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "CommonPlayerController.h"
#include "Camera/BECameraAssistInterface.h"
#include "Teams/BETeamAgentInterface.h"

#include "Containers/Set.h"
#include "Containers/UnrealString.h"
#include "Engine/EngineTypes.h"
#include "GenericTeamAgentInterface.h"
#include "Math/UnrealMathSSE.h"
#include "UObject/UObjectGlobals.h"

#include "BEPlayerController.generated.h"

class ABEHUD;
class ABEPlayerState;
class APawn;
class APlayerState;
class FPrimitiveComponentId;
class IInputInterface;
class UBEAbilitySystemComponent;
class UBESettingsShared;
class UObject;
class UPlayer;
struct FFrame;


/**
 * ABEPlayerController
 *
 *	The base player controller class used by this project.
 */
UCLASS(Config = Game, Meta = (ShortTooltip = "The base player controller class used by this project."))
class BECORE_API ABEPlayerController : public ACommonPlayerController, public IBECameraAssistInterface, public IBETeamAgentInterface
{
	GENERATED_BODY()

public:

	ABEPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "BE|PlayerController")
	ABEPlayerState* GetBEPlayerState() const;

	UFUNCTION(BlueprintCallable, Category = "BE|PlayerController")
	UBEAbilitySystemComponent* GetBEAbilitySystemComponent() const;

	UFUNCTION(BlueprintCallable, Category = "BE|PlayerController")
	ABEHUD* GetBEHUD() const;

	// Run a cheat command on the server.
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerCheat(const FString& Msg);

	// Run a cheat command on the server for all players.
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerCheatAll(const FString& Msg);

	//~AActor interface
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of AActor interface

	//~AController interface
	virtual void OnUnPossess() override;
	//~End of AController interface

	//~APlayerController interface
	virtual void ReceivedPlayer() override;
	virtual void PlayerTick(float DeltaTime) override;
	//~End of APlayerController interface

	//~IBECameraAssistInterface interface
	virtual void OnCameraPenetratingTarget() override;
	//~End of IBECameraAssistInterface interface

	//~ACommonPlayerController interface
	virtual void OnPossess(APawn* InPawn) override;
	//~End of ACommonPlayerController interface
	
	//~IBETeamAgentInterface interface
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual FOnBETeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	//~End of IBETeamAgentInterface interface

private:
	UPROPERTY()
	FOnBETeamIndexChangedDelegate OnTeamChangedDelegate;

	UPROPERTY()
	TObjectPtr<APlayerState> LastSeenPlayerState;

private:
	UFUNCTION()
	void OnPlayerStateChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

protected:
	// Called when the player state is set or cleared
	virtual void OnPlayerStateChanged();

private:
	void BroadcastOnPlayerStateChanged();

protected:
	//~AController interface
	virtual void InitPlayerState() override;
	virtual void CleanupPlayerState() override;
	virtual void OnRep_PlayerState() override;
	//~End of AController interface

	//~APlayerController interface
	virtual void SetPlayer(UPlayer* InPlayer) override;
	virtual void AddCheats(bool bForce) override;

	virtual void UpdateForceFeedback(IInputInterface* InputInterface, const int32 ControllerId) override;
	virtual void UpdateHiddenComponents(const FVector& ViewLocation, TSet<FPrimitiveComponentId>& OutHiddenComponents) override;

	virtual void PreProcessInput(const float DeltaTime, const bool bGamePaused) override;
	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;
	//~End of APlayerController interface

	void OnSettingsChanged(UBESettingsShared* Settings);

	bool bHideViewTargetPawnNextFrame = false;
};


// A player controller used for replay capture and playback
UCLASS()
class ABEReplayPlayerController : public ABEPlayerController
{
	GENERATED_BODY()

	virtual void SetPlayer(UPlayer* InPlayer) override;
};

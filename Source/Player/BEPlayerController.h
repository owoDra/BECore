// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "CommonPlayerController.h"
#include "Camera/BECameraAssistInterface.h"
#include "Team/BETeamAgentInterface.h"

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
 *	このプロジェクトで使用される基本 PlayerController クラス。
 */
UCLASS(Config = Game)
class BECORE_API ABEPlayerController 
	: public ACommonPlayerController
	, public IBECameraAssistInterface
	, public IBETeamAgentInterface
{
	GENERATED_BODY()

public:
	ABEPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	//~AActor interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of AActor interface

	//~ACommonPlayerController interface
	virtual void OnPossess(APawn* InPawn) override;
	//~End of ACommonPlayerController interface

	//~AController interface
	virtual void OnUnPossess() override;
	//~End of AController interface

	//~APlayerController interface
	virtual void ReceivedPlayer() override;
	virtual void PlayerTick(float DeltaTime) override;
	//~End of APlayerController interface

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

	// Called when the player state is set or cleared
	virtual void OnPlayerStateChanged();

	bool bHideViewTargetPawnNextFrame = false;

private:
	void BroadcastOnPlayerStateChanged();

	
public:
	//~IBECameraAssistInterface interface
	virtual void OnCameraPenetratingTarget() override;
	//~End of IBECameraAssistInterface interface


public:
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


public:
	UFUNCTION(BlueprintCallable, Category = "PlayerController")
	ABEPlayerState* GetBEPlayerState() const;

	UFUNCTION(BlueprintCallable, Category = "PlayerController")
	UBEAbilitySystemComponent* GetBEAbilitySystemComponent() const;

	UFUNCTION(BlueprintCallable, Category = "PlayerController")
	ABEHUD* GetBEHUD() const;

	// Run a cheat command on the server.
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerCheat(const FString& Msg);

	// Run a cheat command on the server for all players.
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerCheatAll(const FString& Msg);
};


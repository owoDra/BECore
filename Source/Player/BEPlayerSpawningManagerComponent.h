// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "Components/GameStateComponent.h"
#include "GameFramework/OnlineReplStructs.h"

#include "BEPlayerSpawningManagerComponent.generated.h"

class AController;
class APlayerController;
class APlayerState;
class APlayerStart;
class ABEPlayerStart;
class AActor;

/**
 * @class UBEPlayerSpawningManagerComponent
 */
UCLASS()
class BECORE_API UBEPlayerSpawningManagerComponent : public UGameStateComponent
{
	GENERATED_BODY()

public:
	UBEPlayerSpawningManagerComponent(const FObjectInitializer& ObjectInitializer);

	/** UActorComponent */
	virtual void InitializeComponent() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	/** ~UActorComponent */

protected:
	// Utility
	APlayerStart* GetFirstRandomUnoccupiedPlayerStart(AController* Controller, const TArray<ABEPlayerStart*>& FoundStartPoints) const;
	
	virtual AActor* OnChoosePlayerStart(AController* Player, TArray<ABEPlayerStart*>& PlayerStarts) { return nullptr; }
	virtual void OnFinishRestartPlayer(AController* Player, const FRotator& StartRotation) { }

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName=OnFinishRestartPlayer))
	void K2_OnFinishRestartPlayer(AController* Player, const FRotator& StartRotation);

private:

	/** We proxy these calls from ABEGameMode, to this component so that each experience can more easily customize the respawn system they want. */
	AActor* ChoosePlayerStart(AController* Player);
	bool ControllerCanRestart(AController* Player);
	void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation);
	friend class ABEGameMode;
	/** ~ABEGameMode */

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<ABEPlayerStart>> CachedPlayerStarts;

private:
	void OnLevelAdded(ULevel* InLevel, UWorld* InWorld);
	void HandleOnActorSpawned(AActor* SpawnedActor);

#if WITH_EDITOR
	APlayerStart* FindPlayFromHereStart(AController* Player);
#endif
};

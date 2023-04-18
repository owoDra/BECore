// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "ModularGameState.h"

#include "Messages/BEVerbMessage.h"

#include "AbilitySystemInterface.h"
#include "Engine/EngineTypes.h"
#include "UObject/UObjectGlobals.h"

#include "BEGameState.generated.h"

class APlayerState;
class UBEExperienceManagerComponent;
class UBEAbilitySystemComponent;
class UAbilitySystemComponent;
class UObject;
struct FFrame;


/**
 * ABEGameState
 *
 *	The base game state class used by this project.
 */
UCLASS(Config = Game)
class BECORE_API ABEGameState : public AModularGameStateBase, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:

	ABEGameState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	float GetServerFPS() const { return ServerFPS; }

	//~AActor interface
	virtual void PreInitializeComponents() override;
	virtual void PostInitializeComponents() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of AActor interface

	//~AGameStateBase interface
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;
	//~End of AGameStateBase interface

	//~IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~End of IAbilitySystemInterface

	UFUNCTION(BlueprintCallable, Category = "GameState")
	UBEAbilitySystemComponent* GetBEAbilitySystemComponent() const { return AbilitySystemComponent; }

	// Send a message that all clients will (probably) get
	// (use only for client notifications like eliminations, server join messages, etc... that can handle being lost)
	UFUNCTION(NetMulticast, Unreliable, BlueprintCallable, Category = "GameState")
	void MulticastMessageToClients(const FBEVerbMessage Message);

	// Send a message that all clients will be guaranteed to get
	// (use only for client notifications that cannot handle being lost)
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "GameState")
	void MulticastReliableMessageToClients(const FBEVerbMessage Message);

private:
	UPROPERTY()
	TObjectPtr<UBEExperienceManagerComponent> ExperienceManagerComponent;

	// The ability system component subobject for game-wide things (primarily gameplay cues)
	UPROPERTY(VisibleAnywhere, Category = "GameState")
	TObjectPtr<UBEAbilitySystemComponent> AbilitySystemComponent;


protected:

	virtual void Tick(float DeltaSeconds) override;

protected:
	UPROPERTY(Replicated)
	float ServerFPS;
};

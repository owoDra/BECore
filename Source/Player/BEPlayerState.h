// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "ModularPlayerState.h"

#include "Messages/BEVerbMessage.h"
#include "Teams/BETeamAgentInterface.h"
#include "Ability/BEAbilitySet.h"

#include "AbilitySystemInterface.h"
#include "GenericTeamAgentInterface.h"
#include "HAL/Platform.h"
#include "System/GameplayTagStack.h"
#include "Templates/Casts.h"
#include "UObject/UObjectGlobals.h"

#include "BEPlayerState.generated.h"

class UBECharacterData;
class ABEPlayerController;
class UBEExperienceDefinition;
class UBEAbilitySystemComponent;
class UBEHealthSet;
class UBECombatSet;
class UBEMovementSet;
class UAbilitySystemComponent;
class AController;
class APlayerState;
class FName;
class UObject;
struct FFrame;
struct FGameplayTag;
struct FBEAbilitySet_GrantedHandles;


/** Defines the types of client connected */
UENUM()
enum class EBEPlayerConnectionType : uint8
{
	// An active player
	Player = 0,

	// Spectator connected to a running game
	LiveSpectator,

	// Spectating a demo recording offline
	ReplaySpectator,

	// A deactivated player (disconnected)
	InactivePlayer
};

/**
 * ABEPlayerState
 *
 *	Base player state class used by this project.
 */
UCLASS(Config = Game)
class BECORE_API ABEPlayerState : public AModularPlayerState, public IAbilitySystemInterface, public IBETeamAgentInterface
{
	GENERATED_BODY()

public:
	ABEPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	ABEPlayerController* GetBEPlayerController() const;

	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	UBEAbilitySystemComponent* GetBEAbilitySystemComponent() const { return AbilitySystemComponent; }
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	template <class T>
	const T* GetPawnData() const { return Cast<T>(PawnData); }

	void SetPawnData(const UBECharacterData* InPawnData, bool Override = false);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "PlayerState", meta = (DisplayName = "SetPawnData"))
	void K2_SetPawnData(const UBECharacterData* InPawnData, bool Override = false);

	//~AActor interface
	virtual void PreInitializeComponents() override;
	virtual void PostInitializeComponents() override;
	//~End of AActor interface

	//~APlayerState interface
	virtual void Reset() override;
	virtual void ClientInitialize(AController* C) override;
	virtual void CopyProperties(APlayerState* PlayerState) override;
	virtual void OnDeactivated() override;
	virtual void OnReactivated() override;
	//~End of APlayerState interface

	//~IBETeamAgentInterface interface
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual FOnBETeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	//~End of IBETeamAgentInterface interface

	static const FName NAME_BEAbilityReady;

	void SetPlayerConnectionType(EBEPlayerConnectionType NewType);
	EBEPlayerConnectionType GetPlayerConnectionType() const { return MyPlayerConnectionType; }

	/** Returns the Squad ID of the squad the player belongs to. */
	UFUNCTION(BlueprintCallable)
	int32 GetSquadId() const
	{
		return MySquadID;
	}

	/** Returns the Team ID of the team the player belongs to. */
	UFUNCTION(BlueprintCallable)
	int32 GetTeamId() const
	{
		return GenericTeamIdToInteger(MyTeamID);
	}

	void SetSquadID(int32 NewSquadID);

	// Adds a specified number of stacks to the tag (does nothing if StackCount is below 1)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Teams)
	void AddStatTagStack(FGameplayTag Tag, int32 StackCount);

	// Removes a specified number of stacks from the tag (does nothing if StackCount is below 1)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Teams)
	void RemoveStatTagStack(FGameplayTag Tag, int32 StackCount);

	// Returns the stack count of the specified tag (or 0 if the tag is not present)
	UFUNCTION(BlueprintCallable, Category=Teams)
	int32 GetStatTagStackCount(FGameplayTag Tag) const;

	// Returns true if there is at least one stack of the specified tag
	UFUNCTION(BlueprintCallable, Category=Teams)
	bool HasStatTag(FGameplayTag Tag) const;

	// Send a message to just this player
	// (use only for client notifications like accolades, quest toasts, etc... that can handle being occasionally lost)
	UFUNCTION(Client, Unreliable, BlueprintCallable, Category = "PlayerState")
	void ClientBroadcastMessage(const FBEVerbMessage Message);

private:
	void OnExperienceLoaded(const UBEExperienceDefinition* CurrentExperience);

protected:
	UFUNCTION()
	void OnRep_PawnData();

protected:

	UPROPERTY(ReplicatedUsing = OnRep_PawnData)
	TObjectPtr<const UBECharacterData> PawnData;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBEAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBEHealthSet> HealthSet;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBECombatSet> CombatSet;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBEMovementSet> MovementSet;

	UPROPERTY(Replicated)
	EBEPlayerConnectionType MyPlayerConnectionType;

	UPROPERTY()
	FOnBETeamIndexChangedDelegate OnTeamChangedDelegate;

	UPROPERTY(ReplicatedUsing=OnRep_MyTeamID)
	FGenericTeamId MyTeamID;

	UPROPERTY(ReplicatedUsing=OnRep_MySquadID)
	int32 MySquadID;

	UPROPERTY(Replicated)
	FGameplayTagStackContainer StatTags;

	FBEAbilitySet_GrantedHandles PawnDataAbilityHandles;

private:
	UFUNCTION()
	void OnRep_MyTeamID(FGenericTeamId OldTeamID);

	UFUNCTION()
	void OnRep_MySquadID();
};

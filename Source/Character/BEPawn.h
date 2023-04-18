// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "ModularPawn.h"

#include "Teams/BETeamAgentInterface.h"

#include "Engine/EngineTypes.h"
#include "GenericTeamAgentInterface.h"
#include "HAL/Platform.h"
#include "ModularPawn.h"
#include "UObject/UObjectGlobals.h"

#include "BEPawn.generated.h"

class AController;
class UObject;
struct FFrame;


/**
 * ABEPawn
 */
UCLASS()
class BECORE_API ABEPawn : public AModularPawn, public IBETeamAgentInterface
{
	GENERATED_BODY()

public:

	ABEPawn(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~AActor interface
	virtual void PreInitializeComponents() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of AActor interface

	//~APawn interface
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	//~End of APawn interface

	//~IBETeamAgentInterface interface
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual FOnBETeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	//~End of IBETeamAgentInterface interface

protected:
	// Called to determine what happens to the team ID when possession ends
	virtual FGenericTeamId DetermineNewTeamAfterPossessionEnds(FGenericTeamId OldTeamID) const
	{
		// This could be changed to return, e.g., OldTeamID if you want to keep it assigned afterwards, or return an ID for some neutral faction, or etc...
		return FGenericTeamId::NoTeam;
	}

private:
	UFUNCTION()
	void OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

private:
	UPROPERTY(ReplicatedUsing = OnRep_MyTeamID)
	FGenericTeamId MyTeamID;

	UPROPERTY()
	FOnBETeamIndexChangedDelegate OnTeamChangedDelegate;

private:
	UFUNCTION()
	void OnRep_MyTeamID(FGenericTeamId OldTeamID);
};

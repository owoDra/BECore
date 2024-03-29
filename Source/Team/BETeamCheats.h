// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "GameFramework/CheatManager.h"

#include "HAL/Platform.h"
#include "UObject/UObjectGlobals.h"

#include "BETeamCheats.generated.h"

class UObject;
struct FFrame;


/** Cheats related to teams */
UCLASS()
class UBETeamCheats : public UCheatManagerExtension
{
	GENERATED_BODY()

public:
	// Moves this player to the next available team, wrapping around to the
	// first team if at the end of the list of teams
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void CycleTeam();

	// Moves this player to the specified team
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void SetTeam(int32 TeamID);

	// Prints a list of all of the teams
	UFUNCTION(Exec)
	virtual void ListTeams();
};

// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "CoreMinimal.h"
#include "Components/GameStateComponent.h"

#include "BETeamCreationComponent.generated.h"

class UBEExperienceDefinition;
class ABETeamPublicInfo;
class ABETeamPrivateInfo;
class ABEPlayerState;
class AGameModeBase;
class APlayerController;
class UBETeamDisplayAsset;

UCLASS(Blueprintable)
class UBETeamCreationComponent : public UGameStateComponent
{
	GENERATED_BODY()

public:
	UBETeamCreationComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UObject interface
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
#endif
	//~End of UObject interface

	//~UActorComponent interface
	virtual void BeginPlay() override;
	//~End of UActorComponent interface

private:
	void OnExperienceLoaded(const UBEExperienceDefinition* Experience);

protected:
	// List of teams to create (id to display asset mapping, the display asset can be left unset if desired)
	UPROPERTY(EditDefaultsOnly, Category = Teams)
	TMap<uint8, TObjectPtr<UBETeamDisplayAsset>> TeamsToCreate;

	UPROPERTY(EditDefaultsOnly, Category=Teams)
	TSubclassOf<ABETeamPublicInfo> PublicTeamInfoClass;

	UPROPERTY(EditDefaultsOnly, Category=Teams)
	TSubclassOf<ABETeamPrivateInfo> PrivateTeamInfoClass;

#if WITH_SERVER_CODE
protected:
	virtual void ServerCreateTeams();
	virtual void ServerAssignPlayersToTeams();

	/** Sets the team ID for the given player state. Spectator-only player states will be stripped of any team association. */
	virtual void ServerChooseTeamForPlayer(ABEPlayerState* PS);

private:
	void OnPostLogin(AGameModeBase* GameMode, AController* NewPlayer);
	void ServerCreateTeam(int32 TeamId, UBETeamDisplayAsset* DisplayAsset);

	/** returns the Team ID with the fewest active players, or INDEX_NONE if there are no valid teams */
	int32 GetLeastPopulatedTeamID() const;
#endif
};

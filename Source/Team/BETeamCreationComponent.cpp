// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#include "BETeamCreationComponent.h"

#include "GameMode/Experience/BEExperienceDefinition.h"
#include "GameMode/Experience/BEExperienceManagerComponent.h"
#include "BETeamPublicInfo.h"
#include "BETeamPrivateInfo.h"
#include "Player/BEPlayerState.h"
#include "GameMode/BEGameMode.h"

#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BETeamCreationComponent)


UBETeamCreationComponent::UBETeamCreationComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PublicTeamInfoClass = ABETeamPublicInfo::StaticClass();
	PrivateTeamInfoClass = ABETeamPrivateInfo::StaticClass();
}

#if WITH_EDITOR
EDataValidationResult UBETeamCreationComponent::IsDataValid(TArray<FText>& ValidationErrors)
{
	EDataValidationResult Result = Super::IsDataValid(ValidationErrors);

	//@TODO: TEAMS: Validate that all display assets have the same properties set!

	return Result;
}
#endif

void UBETeamCreationComponent::BeginPlay()
{
	Super::BeginPlay();

	// Listen for the experience load to complete
	AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>();
	UBEExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UBEExperienceManagerComponent>();
	check(ExperienceComponent);
	ExperienceComponent->CallOrRegister_OnExperienceLoaded_HighPriority(FOnBEExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
}

void UBETeamCreationComponent::OnExperienceLoaded(const UBEExperienceDefinition* Experience)
{
#if WITH_SERVER_CODE
	if (HasAuthority())
	{
		ServerCreateTeams();
		ServerAssignPlayersToTeams();
	}
#endif
}

#if WITH_SERVER_CODE

void UBETeamCreationComponent::ServerCreateTeams()
{
	for (const auto& KVP : TeamsToCreate)
	{
		const int32 TeamId = KVP.Key;
		ServerCreateTeam(TeamId, KVP.Value);
	}
}

void UBETeamCreationComponent::ServerAssignPlayersToTeams()
{
	// Assign players that already exist to teams
	AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>();
	for (APlayerState* PS : GameState->PlayerArray)
	{
		if (ABEPlayerState* BEPS = Cast<ABEPlayerState>(PS))
		{
			ServerChooseTeamForPlayer(BEPS);
		}
	}

	// Listen for new players logging in
	ABEGameMode* GameMode = Cast<ABEGameMode>(GameState->AuthorityGameMode);
	check(GameMode);

	GameMode->OnGameModeCombinedPostLogin().AddUObject(this, &ThisClass::OnPostLogin);
}

void UBETeamCreationComponent::ServerChooseTeamForPlayer(ABEPlayerState* PS)
{
	if (PS->IsOnlyASpectator())
	{
		PS->SetGenericTeamId(FGenericTeamId::NoTeam);
	}
	else
	{
		const FGenericTeamId TeamID = IntegerToGenericTeamId(GetLeastPopulatedTeamID());
		PS->SetGenericTeamId(TeamID);
	}
}

void UBETeamCreationComponent::OnPostLogin(AGameModeBase* GameMode, AController* NewPlayer)
{
	check(NewPlayer);
	check(NewPlayer->PlayerState);
	if (ABEPlayerState* BEPS = Cast<ABEPlayerState>(NewPlayer->PlayerState))
	{
		ServerChooseTeamForPlayer(BEPS);
	}
}

void UBETeamCreationComponent::ServerCreateTeam(int32 TeamId, UBETeamDisplayAsset* DisplayAsset)
{
	check(HasAuthority());

	//@TODO: ensure the team doesn't already exist

	UWorld* World = GetWorld();
	check(World);

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ABETeamPublicInfo* NewTeamPublicInfo = World->SpawnActor<ABETeamPublicInfo>(PublicTeamInfoClass, SpawnInfo);
	checkf(NewTeamPublicInfo != nullptr, TEXT("Failed to create public team actor from class %s"), *GetPathNameSafe(*PublicTeamInfoClass));
	NewTeamPublicInfo->SetTeamId(TeamId);
	NewTeamPublicInfo->SetTeamDisplayAsset(DisplayAsset);

	ABETeamPrivateInfo* NewTeamPrivateInfo = World->SpawnActor<ABETeamPrivateInfo>(PrivateTeamInfoClass, SpawnInfo);
	checkf(NewTeamPrivateInfo != nullptr, TEXT("Failed to create private team actor from class %s"), *GetPathNameSafe(*PrivateTeamInfoClass));
	NewTeamPrivateInfo->SetTeamId(TeamId);
}

int32 UBETeamCreationComponent::GetLeastPopulatedTeamID() const
{
	const int32 NumTeams = TeamsToCreate.Num();
	if (NumTeams > 0)
	{
		TMap<int32, uint32> TeamMemberCounts;
		TeamMemberCounts.Reserve(NumTeams);

		for (const auto& KVP : TeamsToCreate)
		{
			const int32 TeamId = KVP.Key;
			TeamMemberCounts.Add(TeamId, 0);
		}

		AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>();
		for (APlayerState* PS : GameState->PlayerArray)
		{
			if (ABEPlayerState* BEPS = Cast<ABEPlayerState>(PS))
			{
				const int32 PlayerTeamID = BEPS->GetTeamId();

				if ((PlayerTeamID != INDEX_NONE) && !BEPS->IsInactive())	// do not count unassigned or disconnected players
				{
					check(TeamMemberCounts.Contains(PlayerTeamID))
					TeamMemberCounts[PlayerTeamID] += 1;
				}
			}
		}

		// sort by lowest team population, then by team ID
		int32 BestTeamId = INDEX_NONE;
		uint32 BestPlayerCount = TNumericLimits<uint32>::Max();
		for (const auto& KVP : TeamMemberCounts)
		{
			const int32 TestTeamId = KVP.Key;
			const uint32 TestTeamPlayerCount = KVP.Value;

			if (TestTeamPlayerCount < BestPlayerCount)
			{
				BestTeamId = TestTeamId;
				BestPlayerCount = TestTeamPlayerCount;
			}
			else if (TestTeamPlayerCount == BestPlayerCount)
			{
				if ((TestTeamId < BestTeamId) || (BestTeamId == INDEX_NONE))
				{
					BestTeamId = TestTeamId;
					BestPlayerCount = TestTeamPlayerCount;
				}
			}
		}

		return BestTeamId;
	}

	return INDEX_NONE;
}
#endif	// WITH_SERVER_CODE

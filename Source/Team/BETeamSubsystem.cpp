// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BETeamSubsystem.h"

#include "BELogChannels.h"
#include "Team/BETeamAgentInterface.h"
#include "Team/BETeamCheats.h"
#include "Team/BETeamInfoBase.h"
#include "Team/BETeamPrivateInfo.h"
#include "Team/BETeamPublicInfo.h"
#include "Player/BEPlayerState.h"

#include "AbilitySystemGlobals.h"
#include "Containers/UnrealString.h"
#include "CoreTypes.h"
#include "GameFramework/Actor.h"
#include "GameFramework/CheatManager.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameplayTagContainer.h"
#include "GenericTeamAgentInterface.h"
#include "Logging/LogCategory.h"
#include "Logging/LogMacros.h"
#include "Misc/AssertionMacros.h"
#include "System/GameplayTagStack.h"
#include "Templates/Casts.h"
#include "Templates/Tuple.h"
#include "Trace/Detail/Channel.h"
#include "UObject/UObjectBaseUtility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BETeamSubsystem)

class FSubsystemCollectionBase;


//////////////////////////////////////////////////////////////////////
// FBETeamTrackingInfo

void FBETeamTrackingInfo::SetTeamInfo(ABETeamInfoBase* Info)
{
	if (ABETeamPublicInfo* NewPublicInfo = Cast<ABETeamPublicInfo>(Info))
	{
		ensure((PublicInfo == nullptr) || (PublicInfo == NewPublicInfo));
		PublicInfo = NewPublicInfo;

		UBETeamDisplayAsset* OldDisplayAsset = DisplayAsset;
		DisplayAsset = NewPublicInfo->GetTeamDisplayAsset();

		if (OldDisplayAsset != DisplayAsset)
		{
			OnTeamDisplayAssetChanged.Broadcast(DisplayAsset);
		}
	}
	else if (ABETeamPrivateInfo* NewPrivateInfo = Cast<ABETeamPrivateInfo>(Info))
	{
		ensure((PrivateInfo == nullptr) || (PrivateInfo == NewPrivateInfo));
		PrivateInfo = NewPrivateInfo;
	}
	else
	{
		checkf(false, TEXT("Expected a public or private team info but got %s"), *GetPathNameSafe(Info))
	}
}

void FBETeamTrackingInfo::RemoveTeamInfo(ABETeamInfoBase* Info)
{
	if (PublicInfo == Info)
	{
		PublicInfo = nullptr;
	}
	else if (PrivateInfo == Info)
	{
		PrivateInfo = nullptr;
	}
	else
	{
		ensureMsgf(false, TEXT("Expected a previously registered team info but got %s"), *GetPathNameSafe(Info));
	}
}

//////////////////////////////////////////////////////////////////////
// UBETeamSubsystem

UBETeamSubsystem::UBETeamSubsystem()
{
}

void UBETeamSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	auto AddTeamCheats = [](UCheatManager* CheatManager)
	{
		CheatManager->AddCheatManagerExtension(NewObject<UBETeamCheats>(CheatManager));
	};

	CheatManagerRegistrationHandle = UCheatManager::RegisterForOnCheatManagerCreated(FOnCheatManagerCreated::FDelegate::CreateLambda(AddTeamCheats));
}

void UBETeamSubsystem::Deinitialize()
{
	UCheatManager::UnregisterFromOnCheatManagerCreated(CheatManagerRegistrationHandle);

	Super::Deinitialize();
}

void UBETeamSubsystem::RegisterTeamInfo(ABETeamInfoBase* TeamInfo)
{
	check(TeamInfo);
	const int32 TeamId = TeamInfo->GetTeamId();
	check(TeamId != INDEX_NONE);

	FBETeamTrackingInfo& Entry = TeamMap.FindOrAdd(TeamId);
	Entry.SetTeamInfo(TeamInfo);
}

void UBETeamSubsystem::UnregisterTeamInfo(ABETeamInfoBase* TeamInfo)
{
	check(TeamInfo);
	const int32 TeamId = TeamInfo->GetTeamId();
	check(TeamId != INDEX_NONE);

	FBETeamTrackingInfo& Entry = TeamMap.FindChecked(TeamId);
	Entry.RemoveTeamInfo(TeamInfo);
}

bool UBETeamSubsystem::ChangeTeamForActor(AActor* ActorToChange, int32 NewTeamIndex)
{
	const FGenericTeamId NewTeamID = IntegerToGenericTeamId(NewTeamIndex);
	if (ABEPlayerState* BEPS = const_cast<ABEPlayerState*>(FindPlayerStateFromActor(ActorToChange)))
	{
		BEPS->SetGenericTeamId(NewTeamID);
		return true;
	}
	else if (IBETeamAgentInterface* TeamActor = Cast<IBETeamAgentInterface>(ActorToChange))
	{
		TeamActor->SetGenericTeamId(NewTeamID);
		return true;
	}
	else
	{
		return false;
	}
}

int32 UBETeamSubsystem::FindTeamFromObject(const UObject* TestObject) const
{
	// See if it's directly a team agent
	if (const IBETeamAgentInterface* ObjectWithTeamInterface = Cast<IBETeamAgentInterface>(TestObject))
	{
		return GenericTeamIdToInteger(ObjectWithTeamInterface->GetGenericTeamId());
	}

	if (const AActor* TestActor = Cast<const AActor>(TestObject))
	{
		// See if the instigator is a team actor
		if (const IBETeamAgentInterface* InstigatorWithTeamInterface = Cast<IBETeamAgentInterface>(TestActor->GetInstigator()))
		{
			return GenericTeamIdToInteger(InstigatorWithTeamInterface->GetGenericTeamId());
		}

		// Fall back to finding the associated player state
		if (const ABEPlayerState* BEPS = FindPlayerStateFromActor(TestActor))
		{
			return BEPS->GetTeamId();
		}
	}

	return INDEX_NONE;
}

const ABEPlayerState* UBETeamSubsystem::FindPlayerStateFromActor(const AActor* PossibleTeamActor) const
{
	if (PossibleTeamActor != nullptr)
	{
		if (const APawn* Pawn = Cast<const APawn>(PossibleTeamActor))
		{
			//@TODO: Consider an interface instead or have team actors register with the subsystem and have it maintain a map? (or LWC style)
			if (ABEPlayerState* BEPS = Pawn->GetPlayerState<ABEPlayerState>())
			{
				return BEPS;
			}
		}
		else if (const AController* PC = Cast<const AController>(PossibleTeamActor))
		{
			if (ABEPlayerState* BEPS = Cast<ABEPlayerState>(PC->PlayerState))
			{
				return BEPS;
			}
		}
		else if (const ABEPlayerState* BEPS = Cast<const ABEPlayerState>(PossibleTeamActor))
		{
			return BEPS; 
		}

		// Try the instigator
// 		if (AActor* Instigator = PossibleTeamActor->GetInstigator())
// 		{
// 			if (ensure(Instigator != PossibleTeamActor))
// 			{
// 				return FindPlayerStateFromActor(Instigator);
// 			}
// 		}
	}

	return nullptr;
}

EBETeamComparison UBETeamSubsystem::CompareTeams(const UObject* A, const UObject* B, int32& TeamIdA, int32& TeamIdB) const
{
	TeamIdA = FindTeamFromObject(Cast<const AActor>(A));
	TeamIdB = FindTeamFromObject(Cast<const AActor>(B));

	if ((TeamIdA == INDEX_NONE) || (TeamIdB == INDEX_NONE))
	{
		return EBETeamComparison::InvalidArgument;
	}
	else
	{
		return (TeamIdA == TeamIdB) ? EBETeamComparison::OnSameTeam : EBETeamComparison::DifferentTeams;
	}
}

EBETeamComparison UBETeamSubsystem::CompareTeams(const UObject* A, const UObject* B) const
{
	int32 TeamIdA;
	int32 TeamIdB;
	return CompareTeams(A, B, /*out*/ TeamIdA, /*out*/ TeamIdB);
}

void UBETeamSubsystem::FindTeamFromActor(const UObject* TestObject, bool& bIsPartOfTeam, int32& TeamId) const
{
	TeamId = FindTeamFromObject(TestObject);
	bIsPartOfTeam = TeamId != INDEX_NONE;
}

void UBETeamSubsystem::AddTeamTagStack(int32 TeamId, FGameplayTag Tag, int32 StackCount)
{
	auto FailureHandler = [&](const FString& ErrorMessage)
	{
		UE_LOG(LogBETeams, Error, TEXT("AddTeamTagStack(TeamId: %d, Tag: %s, StackCount: %d) %s"), TeamId, *Tag.ToString(), StackCount, *ErrorMessage);
	};

	if (FBETeamTrackingInfo* Entry = TeamMap.Find(TeamId))
	{
		if (Entry->PublicInfo)
		{
			if (Entry->PublicInfo->HasAuthority())
			{
				Entry->PublicInfo->TeamTags.AddStack(Tag, StackCount);
			}
			else
			{
				FailureHandler(TEXT("failed because it was called on a client"));
			}
		}
		else
		{
			FailureHandler(TEXT("failed because there is no team info spawned yet (called too early, before the experience was ready)"));
		}
	}
	else
	{
		FailureHandler(TEXT("failed because it was passed an unknown team id"));
	}
}

void UBETeamSubsystem::RemoveTeamTagStack(int32 TeamId, FGameplayTag Tag, int32 StackCount)
{
	auto FailureHandler = [&](const FString& ErrorMessage)
	{
		UE_LOG(LogBETeams, Error, TEXT("RemoveTeamTagStack(TeamId: %d, Tag: %s, StackCount: %d) %s"), TeamId, *Tag.ToString(), StackCount, *ErrorMessage);
	};

	if (FBETeamTrackingInfo* Entry = TeamMap.Find(TeamId))
	{
		if (Entry->PublicInfo)
		{
			if (Entry->PublicInfo->HasAuthority())
			{
				Entry->PublicInfo->TeamTags.RemoveStack(Tag, StackCount);
			}
			else
			{
				FailureHandler(TEXT("failed because it was called on a client"));
			}
		}
		else
		{
			FailureHandler(TEXT("failed because there is no team info spawned yet (called too early, before the experience was ready)"));
		}
	}
	else
	{
		FailureHandler(TEXT("failed because it was passed an unknown team id"));
	}
}

int32 UBETeamSubsystem::GetTeamTagStackCount(int32 TeamId, FGameplayTag Tag) const
{
	if (const FBETeamTrackingInfo* Entry = TeamMap.Find(TeamId))
	{
		const int32 PublicStackCount = (Entry->PublicInfo != nullptr) ? Entry->PublicInfo->TeamTags.GetStackCount(Tag) : 0;
		const int32 PrivateStackCount = (Entry->PrivateInfo != nullptr) ? Entry->PrivateInfo->TeamTags.GetStackCount(Tag) : 0;
		return PublicStackCount + PrivateStackCount;
	}
	else
	{
		UE_LOG(LogBETeams, Verbose, TEXT("GetTeamTagStackCount(TeamId: %d, Tag: %s) failed because it was passed an unknown team id"), TeamId, *Tag.ToString());
		return 0;
	}
}

bool UBETeamSubsystem::TeamHasTag(int32 TeamId, FGameplayTag Tag) const
{
	return GetTeamTagStackCount(TeamId, Tag) > 0;
}

bool UBETeamSubsystem::DoesTeamExist(int32 TeamId) const
{
	return TeamMap.Contains(TeamId);
}

TArray<int32> UBETeamSubsystem::GetTeamIDs() const
{
	TArray<int32> Result;
	TeamMap.GenerateKeyArray(Result);
	Result.Sort();
	return Result;
}

bool UBETeamSubsystem::CanCauseDamage(const UObject* Instigator, const UObject* Target, bool bAllowDamageToSelf) const
{
	if (bAllowDamageToSelf)
	{
		if ((Instigator == Target) || (FindPlayerStateFromActor(Cast<AActor>(Instigator)) == FindPlayerStateFromActor(Cast<AActor>(Target))))
		{
			return true;
		}
	}

	int32 InstigatorTeamId;
	int32 TargetTeamId;
	const EBETeamComparison Relationship = CompareTeams(Instigator, Target, /*out*/ InstigatorTeamId, /*out*/ TargetTeamId);
	if (Relationship == EBETeamComparison::DifferentTeams)
	{
		return true;
	}
	else if (Relationship == EBETeamComparison::InvalidArgument)
	{
		return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Cast<AActor>(Target)) != nullptr;
	}

	return false;
}

UBETeamDisplayAsset* UBETeamSubsystem::GetTeamDisplayAsset(int32 TeamId, int32 ViewerTeamId)
{
	// Currently ignoring ViewerTeamId

	if (FBETeamTrackingInfo* Entry = TeamMap.Find(TeamId))
	{
		return Entry->DisplayAsset;
	}

	return nullptr;
}

UBETeamDisplayAsset* UBETeamSubsystem::GetEffectiveTeamDisplayAsset(int32 TeamId, UObject* ViewerTeamAgent)
{
	return GetTeamDisplayAsset(TeamId, FindTeamFromObject(ViewerTeamAgent));
}

TArray<int32> UBETeamSubsystem::GetEnemyTeamIDsFromActor(const UObject* TestActor) const
{
	TArray<int32> Result;
	TeamMap.GenerateKeyArray(Result);

	int32 AlliesTeamId = FindTeamFromObject(TestActor);
	if (AlliesTeamId == INDEX_NONE)
	{
		Result.Sort();

		return Result;
	}

	Result.RemoveSwap(AlliesTeamId);
	
	Result.Sort();

	return Result;
}

void UBETeamSubsystem::NotifyTeamDisplayAssetModified(UBETeamDisplayAsset* /*ModifiedAsset*/)
{
	// Broadcasting to all observers when a display asset is edited right now, instead of only the edited one
	for (const auto& KVP : TeamMap)
	{
		const int32 TeamId = KVP.Key;
		const FBETeamTrackingInfo& TrackingInfo = KVP.Value;

		TrackingInfo.OnTeamDisplayAssetChanged.Broadcast(TrackingInfo.DisplayAsset);
	}
}

FOnBETeamDisplayAssetChangedDelegate& UBETeamSubsystem::GetTeamDisplayAssetChangedDelegate(int32 TeamId)
{
	return TeamMap.FindOrAdd(TeamId).OnTeamDisplayAssetChanged;
}

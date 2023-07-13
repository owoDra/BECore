// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#include "BETeamPublicInfo.h"

#include "Containers/Array.h"
#include "Misc/AssertionMacros.h"
#include "Net/UnrealNetwork.h"
#include "UObject/CoreNetTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BETeamPublicInfo)

class FLifetimeProperty;


ABETeamPublicInfo::ABETeamPublicInfo(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ABETeamPublicInfo::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, TeamDisplayAsset, COND_InitialOnly);
}

void ABETeamPublicInfo::SetTeamDisplayAsset(TObjectPtr<UBETeamDisplayAsset> NewDisplayAsset)
{
	check(HasAuthority());
	check(TeamDisplayAsset == nullptr);

	TeamDisplayAsset = NewDisplayAsset;

	TryRegisterWithTeamSubsystem();
}

void ABETeamPublicInfo::OnRep_TeamDisplayAsset()
{
	TryRegisterWithTeamSubsystem();
}

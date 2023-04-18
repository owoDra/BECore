// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "BETeamInfoBase.h"

#include "UObject/ObjectPtr.h"
#include "UObject/UObjectGlobals.h"

#include "BETeamPublicInfo.generated.h"

class UBETeamCreationComponent;
class UBETeamDisplayAsset;
class UObject;
struct FFrame;


UCLASS()
class ABETeamPublicInfo : public ABETeamInfoBase
{
	GENERATED_BODY()

	friend UBETeamCreationComponent;

public:
	ABETeamPublicInfo(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UBETeamDisplayAsset* GetTeamDisplayAsset() const { return TeamDisplayAsset; }

private:
	UFUNCTION()
	void OnRep_TeamDisplayAsset();

	void SetTeamDisplayAsset(TObjectPtr<UBETeamDisplayAsset> NewDisplayAsset);

private:
	UPROPERTY(ReplicatedUsing=OnRep_TeamDisplayAsset)
	TObjectPtr<UBETeamDisplayAsset> TeamDisplayAsset;
};

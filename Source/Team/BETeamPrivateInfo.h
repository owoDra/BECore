// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "BETeamInfoBase.h"

#include "UObject/UObjectGlobals.h"

#include "BETeamPrivateInfo.generated.h"

class UObject;


UCLASS()
class ABETeamPrivateInfo : public ABETeamInfoBase
{
	GENERATED_BODY()

public:
	ABETeamPrivateInfo(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};

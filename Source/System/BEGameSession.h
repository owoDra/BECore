// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#pragma once

#include "GameFramework/GameSession.h"

#include "UObject/UObjectGlobals.h"

#include "BEGameSession.generated.h"

class UObject;


UCLASS(Config = Game)
class ABEGameSession : public AGameSession
{
	GENERATED_BODY()

public:

	ABEGameSession(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	virtual void HandleMatchHasStarted() override;
	virtual void HandleMatchHasEnded() override;
};

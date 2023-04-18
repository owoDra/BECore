// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BEGameSession.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEGameSession)


ABEGameSession::ABEGameSession(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ABEGameSession::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
}

void ABEGameSession::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();
}

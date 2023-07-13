// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#include "BEGameEngine.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEGameEngine)

class IEngineLoop;


UBEGameEngine::UBEGameEngine(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UBEGameEngine::Init(IEngineLoop* InEngineLoop)
{
	Super::Init(InEngineLoop);
}

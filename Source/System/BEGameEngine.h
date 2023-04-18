// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "Engine/GameEngine.h"
#include "UObject/UObjectGlobals.h"

#include "BEGameEngine.generated.h"

class IEngineLoop;
class UObject;


UCLASS()
class UBEGameEngine : public UGameEngine
{
	GENERATED_BODY()

public:

	UBEGameEngine(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	virtual void Init(IEngineLoop* InEngineLoop) override;
};

// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "Engine/DebugCameraController.h"

#include "UObject/UObjectGlobals.h"

#include "BEDebugCameraController.generated.h"

class UObject;


/**
 * ABEDebugCameraController
 *
 *	Used for controlling the debug camera when it is enabled via the cheat manager.
 */
UCLASS()
class ABEDebugCameraController : public ADebugCameraController
{
	GENERATED_BODY()

public:

	ABEDebugCameraController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	virtual void AddCheats(bool bForce) override;
};

// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "Camera/PlayerCameraManager.h"

#include "UObject/UObjectGlobals.h"

#include "BEPlayerCameraManager.generated.h"

#define BE_CAMERA_DEFAULT_FOV		(90.0f)
#define BE_CAMERA_DEFAULT_PITCH_MIN	(-89.0f)
#define BE_CAMERA_DEFAULT_PITCH_MAX	(89.0f)

class FDebugDisplayInfo;
class UCanvas;
class UObject;
class UBEUICameraManagerComponent;


/**
 * ABEPlayerCameraManager
 *
 *	このプロジェクトにおける基本の Player Camera Manager
 */
UCLASS(notplaceable, MinimalAPI)
class ABEPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:

	ABEPlayerCameraManager(const FObjectInitializer& ObjectInitializer);

	UBEUICameraManagerComponent* GetUICameraComponent() const;

protected:

	virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime) override;

	virtual void DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;

private:
	/** The UI Camera Component, controls the camera when UI is doing something important that gameplay doesn't get priority over. */
	UPROPERTY(Transient)
	TObjectPtr<UBEUICameraManagerComponent> UICamera;
};

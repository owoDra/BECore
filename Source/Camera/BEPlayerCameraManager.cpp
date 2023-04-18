// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BEPlayerCameraManager.h"

#include "BECameraComponent.h"
#include "BEUICameraManagerComponent.h"

#include "Containers/UnrealString.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "HAL/Platform.h"
#include "Math/Color.h"
#include "Misc/AssertionMacros.h"
#include "UObject/NameTypes.h"
#include "UObject/ObjectPtr.h"
#include "UObject/UObjectBaseUtility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEPlayerCameraManager)

static FName UICameraComponentName(TEXT("UICamera"));


ABEPlayerCameraManager::ABEPlayerCameraManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DefaultFOV = BE_CAMERA_DEFAULT_FOV;
	ViewPitchMin = BE_CAMERA_DEFAULT_PITCH_MIN;
	ViewPitchMax = BE_CAMERA_DEFAULT_PITCH_MAX;

	UICamera = CreateDefaultSubobject<UBEUICameraManagerComponent>(UICameraComponentName);
}

UBEUICameraManagerComponent* ABEPlayerCameraManager::GetUICameraComponent() const
{
	return UICamera;
}

void ABEPlayerCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
	// If the UI Camera is looking at something, let it have priority.
	if (UICamera->NeedsToUpdateViewTarget())
	{
		Super::UpdateViewTarget(OutVT, DeltaTime);
		UICamera->UpdateViewTarget(OutVT, DeltaTime);
		return;
	}

	Super::UpdateViewTarget(OutVT, DeltaTime);
}

void ABEPlayerCameraManager::DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos)
{
	check(Canvas);

	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	DisplayDebugManager.SetFont(GEngine->GetSmallFont());
	DisplayDebugManager.SetDrawColor(FColor::Yellow);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("BEPlayerCameraManager: %s"), *GetNameSafe(this)));

	Super::DisplayDebug(Canvas, DebugDisplay, YL, YPos);

	const APawn* Pawn = (PCOwner ? PCOwner->GetPawn() : nullptr);

	if (const UBECameraComponent* CameraComponent = UBECameraComponent::FindCameraComponent(Pawn))
	{
		CameraComponent->DrawDebug(Canvas);
	}
}

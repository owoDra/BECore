// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#include "BEDebugCameraController.h"
#include "Development/BEDeveloperCheatManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEDebugCameraController)


ABEDebugCameraController::ABEDebugCameraController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Use the same cheat class as BEPlayerController to allow toggling the debug camera through cheats.
	CheatClass = UBEDeveloperCheatManager::StaticClass();
}

void ABEDebugCameraController::AddCheats(bool bForce)
{
	// Mirrors BEPlayerController's AddCheats() to avoid the player becoming stuck in the debug camera.
#if USING_CHEAT_MANAGER
	Super::AddCheats(true);
#else //#if USING_CHEAT_MANAGER
	Super::AddCheats(bForce);
#endif // #else //#if USING_CHEAT_MANAGER
}

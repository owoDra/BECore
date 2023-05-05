// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BEGameInstance.h"

#include "Player/BEPlayerController.h"
#include "Character/BEPawnInitializeTags.h"

#include "CommonSessionSubsystem.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/LocalPlayer.h"
#include "GameplayTagContainer.h"
#include "Misc/AssertionMacros.h"
#include "Templates/Casts.h"
#include "Engine/LocalPlayer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEGameInstance)


UBEGameInstance::UBEGameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UBEGameInstance::Init()
{
	Super::Init();

	// Register our custom init states
	UGameFrameworkComponentManager* ComponentManager = GetSubsystem<UGameFrameworkComponentManager>(this);

	if (ensure(ComponentManager))
	{
		ComponentManager->RegisterInitState(TAG_InitState_Spawned, false, FGameplayTag());
		ComponentManager->RegisterInitState(TAG_InitState_DataAvailable, false, TAG_InitState_Spawned);
		ComponentManager->RegisterInitState(TAG_InitState_DataInitialized, false, TAG_InitState_DataAvailable);
		ComponentManager->RegisterInitState(TAG_InitState_GameplayReady, false, TAG_InitState_DataInitialized);
	}
}

void UBEGameInstance::Shutdown()
{
	Super::Shutdown();
}

ABEPlayerController* UBEGameInstance::GetPrimaryPlayerController() const
{
	return Cast<ABEPlayerController>(Super::GetPrimaryPlayerController(false));
}

bool UBEGameInstance::CanJoinRequestedSession() const
{
	// Temporary first pass:  Always return true
	// This will be fleshed out to check the player's state
	if (!Super::CanJoinRequestedSession())
	{
		return false;
	}
	return true;
}

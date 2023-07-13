// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#include "BEGameViewportClient.h"

#include "GameplayTag/BETags_Platform.h"

#include "CommonUISettings.h"
#include "GameplayTagContainer.h"
#include "ICommonUIModule.h"
#include "UObject/NameTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEGameViewportClient)

class UGameInstance;


UBEGameViewportClient::UBEGameViewportClient()
	: Super(FObjectInitializer::Get())
{
}

void UBEGameViewportClient::Init(struct FWorldContext& WorldContext, UGameInstance* OwningGameInstance, bool bCreateNewAudioDevice)
{
	Super::Init(WorldContext, OwningGameInstance, bCreateNewAudioDevice);
	
	// We have software cursors set up in our project settings for console/mobile use, but on desktop we're fine with
	// the standard hardware cursors
	const bool UseHardwareCursor = ICommonUIModule::GetSettings().GetPlatformTraits().HasTag(TAG_Platform_Trait_Input_HardwareCursor);
	SetUseSoftwareCursorWidgets(!UseHardwareCursor);
}
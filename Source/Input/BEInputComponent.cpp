// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BEInputComponent.h"

#include "Input/BEMappableConfigPair.h"
#include "Player/BELocalPlayer.h"
#include "Settings/BESettingsLocal.h"

#include "EnhancedInputSubsystems.h"
#include "InputCoreTypes.h"
#include "UObject/NameTypes.h"
#include "UObject/UnrealNames.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEInputComponent)

class UBEInputConfig;


UBEInputComponent::UBEInputComponent(const FObjectInitializer& ObjectInitializer)
{
}

void UBEInputComponent::AddInputMappings(const UBEInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	UBELocalPlayer* LocalPlayer = InputSubsystem->GetLocalPlayer<UBELocalPlayer>();
	check(LocalPlayer);

	// Add any registered input mappings from the settings!
	if (UBESettingsLocal* LocalSettings = UBESettingsLocal::Get())
	{
		// Tell enhanced input about any custom keymappings that the player may have customized
		for (const TPair<FName, FKey>& Pair : LocalSettings->GetCustomPlayerInputConfig())
		{
			if (Pair.Key != NAME_None && Pair.Value.IsValid())
			{
				InputSubsystem->AddPlayerMappedKey(Pair.Key, Pair.Value);
			}
		}
	}
}

void UBEInputComponent::RemoveInputMappings(const UBEInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	UBELocalPlayer* LocalPlayer = InputSubsystem->GetLocalPlayer<UBELocalPlayer>();
	check(LocalPlayer);
	
	if (UBESettingsLocal* LocalSettings = UBESettingsLocal::Get())
	{
		// Remove any registered input contexts
		const TArray<FLoadedMappableConfigPair>& Configs = LocalSettings->GetAllRegisteredInputConfigs();
		for (const FLoadedMappableConfigPair& Pair : Configs)
		{
			InputSubsystem->RemovePlayerMappableConfig(Pair.Config);
		}
		
		// Clear any player mapped keys from enhanced input
		for (const TPair<FName, FKey>& Pair : LocalSettings->GetCustomPlayerInputConfig())
		{
			InputSubsystem->RemovePlayerMappedKey(Pair.Key);
		}
	}
}

void UBEInputComponent::RemoveBinds(TArray<uint32>& BindHandles)
{
	for (uint32 Handle : BindHandles)
	{
		RemoveBindingByHandle(Handle);
	}
	BindHandles.Reset();
}

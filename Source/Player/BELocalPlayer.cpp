// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BELocalPlayer.h"

#include "GameSetting/BEGameDeviceSettings.h"
#include "GameSetting/BEGameSharedSettings.h"

#include "AudioMixerBlueprintLibrary.h"
#include "Delegates/Delegate.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Templates/Casts.h"
#include "UObject/NameTypes.h"
#include "UObject/ObjectPtr.h"
#include "UObject/WeakObjectPtr.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BELocalPlayer)


void UBELocalPlayer::PostInitProperties()
{
	Super::PostInitProperties();

	if (UBEGameDeviceSettings* LocalSettings = GetDeviceSettings())
	{
		LocalSettings->OnAudioOutputDeviceChanged.AddUObject(this, &UBELocalPlayer::OnAudioOutputDeviceChanged);
	}
}

void UBELocalPlayer::SwitchController(class APlayerController* PC)
{
	Super::SwitchController(PC);

	OnPlayerControllerChanged(PlayerController);
}

bool UBELocalPlayer::SpawnPlayActor(const FString& URL, FString& OutError, UWorld* InWorld)
{
	const bool bResult = Super::SpawnPlayActor(URL, OutError, InWorld);

	OnPlayerControllerChanged(PlayerController);

	return bResult;
}

void UBELocalPlayer::InitOnlineSession()
{
	OnPlayerControllerChanged(PlayerController);

	Super::InitOnlineSession();
}


UBEGameDeviceSettings* UBELocalPlayer::GetDeviceSettings() const
{
	return UBEGameDeviceSettings::Get();
}

UBEGameSharedSettings* UBELocalPlayer::GetSharedSettings() const
{
	if (!GameSharedSettings)
	{
		GameSharedSettings = UBEGameSharedSettings::LoadOrCreateSettings(this);
	}

	return GameSharedSettings;
}


void UBELocalPlayer::OnAudioOutputDeviceChanged(const FString& InAudioOutputDeviceId)
{
	FOnCompletedDeviceSwap DevicesSwappedCallback;
	DevicesSwappedCallback.BindUFunction(this, FName("OnCompletedAudioDeviceSwap"));
	UAudioMixerBlueprintLibrary::SwapAudioOutputDevice(GetWorld(), InAudioOutputDeviceId, DevicesSwappedCallback);
}

void UBELocalPlayer::OnCompletedAudioDeviceSwap(const FSwapAudioOutputResult& SwapResult)
{
	if (SwapResult.Result == ESwapAudioOutputDeviceResultState::Failure)
	{
	}
}


void UBELocalPlayer::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	// Do nothing, we merely observe the team of our associated player controller
}

FGenericTeamId UBELocalPlayer::GetGenericTeamId() const
{
	if (IBETeamAgentInterface* ControllerAsTeamProvider = Cast<IBETeamAgentInterface>(PlayerController))
	{
		return ControllerAsTeamProvider->GetGenericTeamId();
	}
	else
	{
		return FGenericTeamId::NoTeam;
	}
}

FOnBETeamIndexChangedDelegate* UBELocalPlayer::GetOnTeamIndexChangedDelegate()
{
	return &OnTeamChangedDelegate;
}

void UBELocalPlayer::OnPlayerControllerChanged(APlayerController* NewController)
{
	// Stop listening for changes from the old controller
	FGenericTeamId OldTeamID = FGenericTeamId::NoTeam;
	if (IBETeamAgentInterface* ControllerAsTeamProvider = Cast<IBETeamAgentInterface>(LastBoundPC.Get()))
	{
		OldTeamID = ControllerAsTeamProvider->GetGenericTeamId();
		ControllerAsTeamProvider->GetTeamChangedDelegateChecked().RemoveAll(this);
	}

	// Grab the current team ID and listen for future changes
	FGenericTeamId NewTeamID = FGenericTeamId::NoTeam;
	if (IBETeamAgentInterface* ControllerAsTeamProvider = Cast<IBETeamAgentInterface>(NewController))
	{
		NewTeamID = ControllerAsTeamProvider->GetGenericTeamId();
		ControllerAsTeamProvider->GetTeamChangedDelegateChecked().AddDynamic(this, &ThisClass::OnControllerChangedTeam);
		LastBoundPC = NewController;
	}

	ConditionalBroadcastTeamChanged(this, OldTeamID, NewTeamID);
}

void UBELocalPlayer::OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
{
	ConditionalBroadcastTeamChanged(this, IntegerToGenericTeamId(OldTeam), IntegerToGenericTeamId(NewTeam));
}

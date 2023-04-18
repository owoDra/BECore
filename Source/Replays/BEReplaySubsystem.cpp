// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BEReplaySubsystem.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Engine/DemoNetDriver.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEReplaySubsystem)


UBEReplaySubsystem::UBEReplaySubsystem()
{
}

void UBEReplaySubsystem::PlayReplay(UBEReplayListEntry* Replay)
{
	if (Replay != nullptr)
	{
		FString DemoName = Replay->StreamInfo.Name;
		GetGameInstance()->PlayReplay(DemoName);
	}
}

// void UBEReplaySubsystem::DeleteReplay()
// {
//	ReplayStreamer->DeleteFinishedStream(SelectedItem->StreamInfo.Name, FDeleteFinishedStreamCallback::CreateSP(this, &SShooterDemoList::OnDeleteFinishedStreamComplete));
// }

void UBEReplaySubsystem::SeekInActiveReplay(float TimeInSeconds)
{
	if (UDemoNetDriver* DemoDriver = GetDemoDriver())
	{
		DemoDriver->GotoTimeInSeconds(TimeInSeconds);
	}
}

float UBEReplaySubsystem::GetReplayLengthInSeconds() const
{
	if (UDemoNetDriver* DemoDriver = GetDemoDriver())
	{
		return DemoDriver->GetDemoTotalTime();
	}
	return 0.0f;
}

float UBEReplaySubsystem::GetReplayCurrentTime() const
{
	if (UDemoNetDriver* DemoDriver = GetDemoDriver())
	{
		return DemoDriver->GetDemoCurrentTime();
	}
	return 0.0f;
}

UDemoNetDriver* UBEReplaySubsystem::GetDemoDriver() const
{
	if (UWorld* World = GetGameInstance()->GetWorld())
	{
		return World->GetDemoNetDriver();
	}
	return nullptr;
}

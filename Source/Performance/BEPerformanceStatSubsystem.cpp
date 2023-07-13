// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#include "BEPerformanceStatSubsystem.h"

#include "GameMode/BEGameState.h"
#include "Performance/BEPerformanceStatTypes.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/NetConnection.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "HAL/Platform.h"
#include "Templates/Casts.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEPerformanceStatSubsystem)

class FSubsystemCollectionBase;


//////////////////////////////////////////////////////////////////////
// FBEPerformanceStatCache

void FBEPerformanceStatCache::StartCharting()
{
}

void FBEPerformanceStatCache::ProcessFrame(const FFrameData& FrameData)
{
	CachedData = FrameData;
	CachedServerFPS = 0.0f;
	CachedPingMS = 0.0f;
	CachedPacketLossIncomingPercent = 0.0f;
	CachedPacketLossOutgoingPercent = 0.0f;
	CachedPacketRateIncoming = 0.0f;
	CachedPacketRateOutgoing = 0.0f;
	CachedPacketSizeIncoming = 0.0f;
	CachedPacketSizeOutgoing = 0.0f;

	if (UWorld* World = MySubsystem->GetGameInstance()->GetWorld())
	{
		if (const ABEGameState* GameState = World->GetGameState<ABEGameState>())
		{
			CachedServerFPS = GameState->GetServerFPS();
		}

		if (APlayerController* LocalPC = GEngine->GetFirstLocalPlayerController(World))
		{
			if (APlayerState* PS = LocalPC->GetPlayerState<APlayerState>())
			{
				CachedPingMS = PS->GetPingInMilliseconds();
			}

			if (UNetConnection* NetConnection = LocalPC->GetNetConnection())
			{
				const UNetConnection::FNetConnectionPacketLoss& InLoss = NetConnection->GetInLossPercentage();
				CachedPacketLossIncomingPercent = InLoss.GetAvgLossPercentage();
				const UNetConnection::FNetConnectionPacketLoss& OutLoss = NetConnection->GetOutLossPercentage();
				CachedPacketLossOutgoingPercent = OutLoss.GetAvgLossPercentage();

				CachedPacketRateIncoming = NetConnection->InPacketsPerSecond;
				CachedPacketRateOutgoing = NetConnection->OutPacketsPerSecond;

				CachedPacketSizeIncoming = (NetConnection->InPacketsPerSecond != 0) ? (NetConnection->InBytesPerSecond / (float)NetConnection->InPacketsPerSecond) : 0.0f;
				CachedPacketSizeOutgoing = (NetConnection->OutPacketsPerSecond != 0) ? (NetConnection->OutBytesPerSecond / (float)NetConnection->OutPacketsPerSecond) : 0.0f;
			}
		}
	}
}

void FBEPerformanceStatCache::StopCharting()
{
}

double FBEPerformanceStatCache::GetCachedStat(EBEDisplayablePerformanceStat Stat) const
{
	static_assert((int32)EBEDisplayablePerformanceStat::Count == 15, "Need to update this function to deal with new performance stats");
	switch (Stat)
	{
	case EBEDisplayablePerformanceStat::ClientFPS:
		return (CachedData.TrueDeltaSeconds != 0.0) ? (1.0 / CachedData.TrueDeltaSeconds) : 0.0;
	case EBEDisplayablePerformanceStat::ServerFPS:
		return CachedServerFPS;
	case EBEDisplayablePerformanceStat::IdleTime:
		return CachedData.IdleSeconds;
	case EBEDisplayablePerformanceStat::FrameTime:
		return CachedData.TrueDeltaSeconds;
	case EBEDisplayablePerformanceStat::FrameTime_GameThread:
		return CachedData.GameThreadTimeSeconds;
	case EBEDisplayablePerformanceStat::FrameTime_RenderThread:
		return CachedData.RenderThreadTimeSeconds;
	case EBEDisplayablePerformanceStat::FrameTime_RHIThread:
		return CachedData.RHIThreadTimeSeconds;
	case EBEDisplayablePerformanceStat::FrameTime_GPU:
		return CachedData.GPUTimeSeconds;
	case EBEDisplayablePerformanceStat::Ping:
		return CachedPingMS;
	case EBEDisplayablePerformanceStat::PacketLoss_Incoming:
		return CachedPacketLossIncomingPercent;
	case EBEDisplayablePerformanceStat::PacketLoss_Outgoing:
		return CachedPacketLossOutgoingPercent;
	case EBEDisplayablePerformanceStat::PacketRate_Incoming:
		return CachedPacketRateIncoming;
	case EBEDisplayablePerformanceStat::PacketRate_Outgoing:
		return CachedPacketRateOutgoing;
	case EBEDisplayablePerformanceStat::PacketSize_Incoming:
		return CachedPacketSizeIncoming;
	case EBEDisplayablePerformanceStat::PacketSize_Outgoing:
		return CachedPacketSizeOutgoing;
	}

	return 0.0f;
}

//////////////////////////////////////////////////////////////////////
// UBEPerformanceStatSubsystem

void UBEPerformanceStatSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Tracker = MakeShared<FBEPerformanceStatCache>(this);
	GEngine->AddPerformanceDataConsumer(Tracker);
}

void UBEPerformanceStatSubsystem::Deinitialize()
{
	GEngine->RemovePerformanceDataConsumer(Tracker);
	Tracker.Reset();
}

double UBEPerformanceStatSubsystem::GetCachedStat(EBEDisplayablePerformanceStat Stat) const
{
	return Tracker->GetCachedStat(Stat);
}

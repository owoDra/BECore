// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "ChartCreation.h"
#include "BEPerformanceStatTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Templates/SharedPointer.h"
#include "UObject/UObjectGlobals.h"

#include "BEPerformanceStatSubsystem.generated.h"

class UBEPerformanceStatSubsystem;
class FSubsystemCollectionBase;
class UObject;
struct FFrame;


//////////////////////////////////////////////////////////////////////

// Observer which caches the stats for the previous frame
struct FBEPerformanceStatCache : public IPerformanceDataConsumer
{
public:
	FBEPerformanceStatCache(UBEPerformanceStatSubsystem* InSubsystem)
		: MySubsystem(InSubsystem)
	{
	}

	//~IPerformanceDataConsumer interface
	virtual void StartCharting() override;
	virtual void ProcessFrame(const FFrameData& FrameData) override;
	virtual void StopCharting() override;
	//~End of IPerformanceDataConsumer interface

	double GetCachedStat(EBEDisplayablePerformanceStat Stat) const;

protected:
	IPerformanceDataConsumer::FFrameData CachedData;
	UBEPerformanceStatSubsystem* MySubsystem;

	float CachedServerFPS = 0.0f;
	float CachedPingMS = 0.0f;
	float CachedPacketLossIncomingPercent = 0.0f;
	float CachedPacketLossOutgoingPercent = 0.0f;
	float CachedPacketRateIncoming = 0.0f;
	float CachedPacketRateOutgoing = 0.0f;
	float CachedPacketSizeIncoming = 0.0f;
	float CachedPacketSizeOutgoing = 0.0f;
};

//////////////////////////////////////////////////////////////////////

// Subsystem to allow access to performance stats for display purposes
UCLASS(BlueprintType)
class UBEPerformanceStatSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	double GetCachedStat(EBEDisplayablePerformanceStat Stat) const;

	//~USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~End of USubsystem interface

protected:
	TSharedPtr<FBEPerformanceStatCache> Tracker;
};

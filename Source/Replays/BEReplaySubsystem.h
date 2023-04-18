// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/Object.h"

#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "HAL/Platform.h"
#include "Misc/DateTime.h"
#include "Misc/Timespan.h"
#include "NetworkReplayStreaming.h"
#include "UObject/UObjectGlobals.h"

#include "BEReplaySubsystem.generated.h"

class UDemoNetDriver;
struct FFrame;


// An available replay
UCLASS(BlueprintType)
class UBEReplayListEntry : public UObject
{
	GENERATED_BODY()

public:
	FNetworkReplayStreamInfo StreamInfo;

	/** The UI friendly name of the stream */
	UFUNCTION(BlueprintPure, Category=Replays)
	FString GetFriendlyName() const { return StreamInfo.FriendlyName; }

	/** The date and time the stream was recorded */
	UFUNCTION(BlueprintPure, Category=Replays)
	FDateTime GetTimestamp() const { return StreamInfo.Timestamp; }

	/** The duration of the stream in MS */
	UFUNCTION(BlueprintPure, Category=Replays)
	FTimespan GetDuration() const { return FTimespan::FromMilliseconds(StreamInfo.LengthInMS); }

	/** Number of viewers viewing this stream */
	UFUNCTION(BlueprintPure, Category=Replays)
	int32 GetNumViewers() const { return StreamInfo.NumViewers; }

	/** True if the stream is live and the game hasn't completed yet */
	UFUNCTION(BlueprintPure, Category=Replays)
	bool GetIsLive() const { return StreamInfo.bIsLive; }
};

// Results of querying for replays
UCLASS(BlueprintType)
class UBEReplayList : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category=Replays)
	TArray<TObjectPtr<UBEReplayListEntry>> Results;
};

UCLASS()
class UBEReplaySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UBEReplaySubsystem();

	UFUNCTION(BlueprintCallable, Category=Replays)
	void PlayReplay(UBEReplayListEntry* Replay);

	//void DeleteReplay();

	UFUNCTION(BlueprintCallable, Category=Replays)
	void SeekInActiveReplay(float TimeInSeconds);

	UFUNCTION(BlueprintCallable, Category = Replays, BlueprintPure = false)
	float GetReplayLengthInSeconds() const;

	UFUNCTION(BlueprintCallable, Category=Replays, BlueprintPure=false)
	float GetReplayCurrentTime() const;

private:
	UDemoNetDriver* GetDemoDriver() const;
};

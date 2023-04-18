// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "AsyncAction_QueryReplays.h"

#include "BEReplaySubsystem.h"

#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "CoreTypes.h"
#include "GameFramework/PlayerController.h"
#include "Misc/DateTime.h"
#include "Misc/NetworkVersion.h"
#include "NetworkReplayStreaming.h"
#include "Templates/Sorting.h"
#include "UObject/WeakObjectPtr.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AsyncAction_QueryReplays)


UAsyncAction_QueryReplays::UAsyncAction_QueryReplays(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UAsyncAction_QueryReplays* UAsyncAction_QueryReplays::QueryReplays(APlayerController* InPlayerController)
{
	UAsyncAction_QueryReplays* Action = nullptr;

	if (InPlayerController != nullptr)
	{
		Action = NewObject<UAsyncAction_QueryReplays>();
		Action->PlayerController = InPlayerController;
	}

	return Action;
}

void UAsyncAction_QueryReplays::Activate()
{
	ReplayStreamer = FNetworkReplayStreaming::Get().GetFactory().CreateReplayStreamer();

	ResultList = NewObject<UBEReplayList>();
	if (ReplayStreamer.IsValid())
	{
		FNetworkReplayVersion EnumerateStreamsVersion = FNetworkVersion::GetReplayVersion();

		ReplayStreamer->EnumerateStreams(EnumerateStreamsVersion, INDEX_NONE, FString(), TArray<FString>(), FEnumerateStreamsCallback::CreateUObject(this, &ThisClass::OnEnumerateStreamsComplete));
	}
	else
	{
		QueryComplete.Broadcast(ResultList);
	}
}

void UAsyncAction_QueryReplays::OnEnumerateStreamsComplete(const FEnumerateStreamsResult& Result)
{
	for (const FNetworkReplayStreamInfo& StreamInfo : Result.FoundStreams)
	{
		UBEReplayListEntry* NewReplayEntry = NewObject<UBEReplayListEntry>(ResultList);
		NewReplayEntry->StreamInfo = StreamInfo;
		ResultList->Results.Add(NewReplayEntry);
	}

	// Sort demo names by date
	struct FCompareDateTime
	{
		FORCEINLINE bool operator()(const UBEReplayListEntry& A, const UBEReplayListEntry& B) const
		{
			return A.StreamInfo.Timestamp.GetTicks() > B.StreamInfo.Timestamp.GetTicks();
		}
	};

	Sort(ResultList->Results.GetData(), ResultList->Results.Num(), FCompareDateTime());

	QueryComplete.Broadcast(ResultList);
}

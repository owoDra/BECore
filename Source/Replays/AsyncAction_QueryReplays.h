// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "Delegates/Delegate.h"
#include "Templates/SharedPointer.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/WeakObjectPtrTemplates.h"

#include "AsyncAction_QueryReplays.generated.h"

class APlayerController;
class INetworkReplayStreamer;
class UBEReplayList;
class UObject;
struct FEnumerateStreamsResult;
struct FFrame;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQueryReplayAsyncDelegate, UBEReplayList*, Results);


/**
 * Watches for team changes in the specified player controller
 */
UCLASS()
class UAsyncAction_QueryReplays : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UAsyncAction_QueryReplays(const FObjectInitializer& ObjectInitializer);

	// Watches for team changes in the specified player controller
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UAsyncAction_QueryReplays* QueryReplays(APlayerController* PlayerController);

	virtual void Activate() override;

public:
	// Called when the replay query completes
	UPROPERTY(BlueprintAssignable)
	FQueryReplayAsyncDelegate QueryComplete;

private:
	void OnEnumerateStreamsComplete(const FEnumerateStreamsResult& Result);

private:
	UPROPERTY()
	TObjectPtr<UBEReplayList> ResultList;

	TWeakObjectPtr<APlayerController> PlayerController;

	TSharedPtr<INetworkReplayStreamer> ReplayStreamer;
};

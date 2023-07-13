// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#pragma once

#include "Engine/CancellableAsyncAction.h"

#include "CoreTypes.h"
#include "Delegates/Delegate.h"
#include "UObject/Object.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/WeakInterfacePtr.h"
#include "UObject/WeakObjectPtr.h"
#include "UObject/WeakObjectPtrTemplates.h"

#include "AsyncAction_ObserveTeamColors.generated.h"

class IBETeamAgentInterface;
class UBETeamDisplayAsset;
struct FFrame;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FTeamColorObservedAsyncDelegate, bool, bTeamSet, int32, TeamId, const UBETeamDisplayAsset*, DisplayAsset);


/**
 * Watches for team color changes in the specified object
 */
UCLASS()
class UAsyncAction_ObserveTeamColors : public UCancellableAsyncAction
{
	GENERATED_UCLASS_BODY()

public:
	// Watches for team changes on the specified team agent
	//  - It will will fire once immediately to give the current team assignment
	//  - For anything that can ever belong to a team (implements IBETeamAgentInterface),
	//    it will also listen for team assignment changes in the future
	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly="true", Keywords="Watch"))
	static UAsyncAction_ObserveTeamColors* ObserveTeamColors(UObject* TeamAgent);

	//~UBlueprintAsyncActionBase interface
	virtual void Activate() override;
	virtual void SetReadyToDestroy() override;
	//~End of UBlueprintAsyncActionBase interface

public:
	// Called when the team is set or changed
	UPROPERTY(BlueprintAssignable)
	FTeamColorObservedAsyncDelegate OnTeamChanged;

private:
	void BroadcastChange(int32 NewTeam, const UBETeamDisplayAsset* DisplayAsset);

	UFUNCTION()
	void OnWatchedAgentChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

	UFUNCTION()
	void OnDisplayAssetChanged(const UBETeamDisplayAsset* DisplayAsset);

	TWeakInterfacePtr<IBETeamAgentInterface> TeamInterfacePtr;
	TWeakObjectPtr<UObject> TeamInterfaceObj;

	int32 LastBroadcastTeamId = INDEX_NONE;
};

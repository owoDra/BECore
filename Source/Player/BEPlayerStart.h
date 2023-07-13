// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#pragma once

#include "GameFramework/PlayerStart.h"

#include "Engine/EngineTypes.h"
#include "GameplayTagContainer.h"
#include "UObject/ObjectPtr.h"
#include "UObject/UObjectGlobals.h"

#include "BEPlayerStart.generated.h"

class AController;
class UObject;


enum class EBEPlayerStartLocationOccupancy
{
	Empty,
	Partial,
	Full
};


/**
 * ABEPlayerStart
 * 
 * Base player starts that can be used by a lot of modes.
 */
UCLASS(Config = Game)
class BECORE_API ABEPlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:
	ABEPlayerStart(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	const FGameplayTagContainer& GetGameplayTags() { return StartPointTags; }

	EBEPlayerStartLocationOccupancy GetLocationOccupancy(AController* const ControllerPawnToFit) const;

	/** Did this player start get claimed by a controller already? */
	bool IsClaimed() const;

	/** If this PlayerStart was not claimed, claim it for ClTargetingController */
	bool TryClaim(AController* OccupyingController);

protected:
	/** Check if this PlayerStart is still claimed */
	void CheckUnclaimed();

	/** The controller that claimed this PlayerStart */
	UPROPERTY(Transient)
	TObjectPtr<AController> ClTargetingController = nullptr;

	/** Interval in which we'll check if this player start is not colliding with anyone anymore */
	UPROPERTY(EditDefaultsOnly, Category = "Player Start ClTargeting")
	float ExpirationCheckInterval = 1.f;

	/** Tags to identify this player start */
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer StartPointTags;

	/** Handle to track expiration recurring timer */
	FTimerHandle ExpirationTimerHandle;
};

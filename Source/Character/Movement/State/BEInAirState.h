// Copyright owoDra

#pragma once

#include "BEInAirState.generated.h"

USTRUCT(BlueprintType)
struct BECORE_API FInAirState
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ForceUnits = "cm/s"))
	float VerticalVelocity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	bool bJumpRequested = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	bool bJumped = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ForceUnits = "x"))
	float JumpPlayRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ClampMax = 1))
	float GroundPredictionAmount = 1.0f;
};

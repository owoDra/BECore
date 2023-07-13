// Copyright owoDra

#pragma once

#include "BERagdollingState.generated.h"

USTRUCT(BlueprintType)
struct BECORE_API FRagdollingState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0))
	int32 SpeedLimitFrameTimeRemaining{0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float SpeedLimit{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FVector RootBoneVelocity{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ForceUnits = "N"))
	float PullForce{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	bool bGrounded{false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	bool bFacedUpward{false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	bool bPendingFinalization{false};
};

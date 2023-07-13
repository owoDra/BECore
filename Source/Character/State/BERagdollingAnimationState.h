// Copyright owoDra

#pragma once

#include "Animation/PoseSnapshot.h"
#include "BERagdollingAnimationState.generated.h"

USTRUCT(BlueprintType)
struct BECORE_API FRagdollingAnimationState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FPoseSnapshot FinalRagdollPose;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ClampMax = 1, ForceUnits = "x"))
	float FlailPlayRate{1.0f};
};

// Copyright owoDra

#pragma once

#include "BELeanState.generated.h"

USTRUCT(BlueprintType)
struct BECORE_API FLeanState
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = -1, ClampMax = 1))
	float RightAmount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = -1, ClampMax = 1))
	float ForwardAmount = 0.0f;
};

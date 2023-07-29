// Copyright owoDra

#pragma once

#include "BERotateInPlaceState.generated.h"

USTRUCT(BlueprintType)
struct BECORE_API FRotateInPlaceState
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	bool bRotatingLeft = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	bool bRotatingRight = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ForceUnits = "x"))
	float PlayRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	float FootLockBlockAmount = 0.0f;
};

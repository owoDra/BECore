// Copyright owoDra

#pragma once

#include "BETransitionsState.generated.h"

class UAnimSequenceBase;

USTRUCT(BlueprintType)
struct BECORE_API FTransitionsState
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	bool bTransitionsAllowed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	int32 DynamicTransitionsFrameDelay = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	TObjectPtr<UAnimSequenceBase> QueuedDynamicTransitionAnimation = nullptr;
};

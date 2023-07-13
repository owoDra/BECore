﻿// Copyright owoDra

#pragma once

#include "BEMovementBaseState.generated.h"

USTRUCT(BlueprintType)
struct BECORE_API FMovementBaseState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	TObjectPtr<UPrimitiveComponent> Primitive{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FName BoneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	bool bBaseChanged{false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	bool bHasRelativeLocation{false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	bool bHasRelativeRotation{false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FVector Location{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FQuat Rotation{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FRotator DeltaRotation{ForceInit};
};

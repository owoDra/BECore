﻿// Copyright owoDra

#pragma once

#include "BEViewAnimationState.generated.h"

USTRUCT(BlueprintType)
struct BECORE_API FLookState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	bool bReinitializationRequired{true};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float WorldYawAngle{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float YawAngle{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = -90, ClampMax = 90, ForceUnits = "deg"))
	float PitchAngle{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ClampMax = 1))
	float YawForwardAmount{0.5f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ClampMax = 0.5))
	float YawLeftAmount{0.5f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0.5, ClampMax = 1))
	float YawRightAmount{0.5f};
};

USTRUCT(BlueprintType)
struct BE_API FAlsSpineRotationState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	bool bSpineRotationAllowed{false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ClampMax = 1))
	float SpineAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float StartYawAngle{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float TargetYawAngle{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float CurrentYawAngle{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float YawAngle{0.0f};
};

USTRUCT(BlueprintType)
struct BE_API FAlsViewAnimationState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FRotator Rotation{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float YawAngle{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ForceUnits = "deg/s"))
	float YawSpeed{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = -90, ClampMax = 90, ForceUnits = "deg"))
	float PitchAngle{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ClampMax = 1))
	float PitchAmount{0.5f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ClampMax = 1))
	float LookAmount{1.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FAlsSpineRotationState SpineRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FAlsLookState Look;
};

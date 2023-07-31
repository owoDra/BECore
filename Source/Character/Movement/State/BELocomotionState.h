﻿// Copyright owoDra

#pragma once

#include "BELocomotionState.generated.h"

USTRUCT(BlueprintType)
struct BECORE_API FLocomotionState
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	bool bHasInput = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float InputYawAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	bool bHasSpeed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float Speed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FVector Velocity = FVector(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FVector PreviousVelocity = FVector(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float VelocityYawAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FVector Acceleration = FVector(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	bool bMoving = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	bool bRotationTowardsLastInputDirectionBlocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float TargetYawAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float ViewRelativeTargetYawAngle = 0.0f;

	// Used for extra smooth actor rotation, in other cases equal to the regular target yaw angle.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float SmoothTargetYawAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FVector Location = FVector(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FRotator Rotation = FRotator(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FQuat RotationQuaternion = FQuat(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float PreviousYawAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ForceUnits = "deg/s"))
	float YawSpeed = 0.0f;
};
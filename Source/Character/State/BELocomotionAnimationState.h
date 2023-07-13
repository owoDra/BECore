// Copyright owoDra

#pragma once

#include "BELocomotionAnimationState.generated.h"

USTRUCT(BlueprintType)
struct BECORE_API FLocomotionAnimationState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	bool bHasInput{false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float InputYawAngle{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float Speed{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FVector Velocity{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float VelocityYawAngle{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FVector Acceleration{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0))
	float MaxAcceleration{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0))
	float MaxBrakingDeceleration{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	float WalkableFloorZ{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	bool bMoving{false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	bool bMovingSmooth{false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float TargetYawAngle{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FVector Location{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FRotator Rotation{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FQuat RotationQuaternion{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ForceUnits = "deg/s"))
	float YawSpeed{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ForceUnits = "x"))
	float Scale{1.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ForceUnits = "cm"))
	float CapsuleRadius{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ForceUnits = "cm"))
	float CapsuleHalfHeight{0.0f};
};

// Copyright owoDra

#pragma once

#include "State/AlsMovementDirection.h"
#include "BEGroundedState.generated.h"

UENUM(BlueprintType)
enum class EHipsDirection : uint8
{
	Forward,
	Backward,
	LeftForward,
	LeftBackward,
	RightForward,
	RightBackward,
};

USTRUCT(BlueprintType)
struct BECORE_API FVelocityBlendState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	bool bReinitializationRequired{true};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ClampMax = 1))
	float ForwardAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ClampMax = 1))
	float BackwardAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ClampMax = 1))
	float LeftAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ClampMax = 1))
	float RightAmount{0.0f};
};

USTRUCT(BlueprintType)
struct BECORE_API FRotationYawOffsetsState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float ForwardAngle{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float BackwardAngle{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float LeftAngle{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float RightAngle{0.0f};
};

USTRUCT(BlueprintType)
struct BECORE_API FGroundedState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	EAlsHipsDirection HipsDirection{EAlsHipsDirection::Forward};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = -1, ClampMax = 1))
	float HipsDirectionLockAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	bool bPivotActivationRequested{false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	bool bPivotActive{false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FAlsMovementDirectionCache MovementDirection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FAlsVelocityBlendState VelocityBlend;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FAlsRotationYawOffsetsState RotationYawOffsets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ForceUnits = "s"))
	float SprintTime{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = -1, ClampMax = 1))
	float SprintAccelerationAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ClampMax = 1))
	float SprintBlockAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ClampMax = 1))
	float WalkRunBlendAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ClampMax = 1))
	float StrideBlendAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ForceUnits = "x"))
	float StandingPlayRate{1.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ForceUnits = "x"))
	float CrouchingPlayRate{1.0f};
};

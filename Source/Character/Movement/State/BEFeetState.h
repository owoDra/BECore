// Copyright owoDra

#pragma once

#include "Character/Movement/State/BESpringState.h"

#include "BEFeetState.generated.h"

USTRUCT(BlueprintType)
struct BECORE_API FFootState
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ClampMax = 1))
	float IkAmount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ClampMax = 1))
	float LockAmount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FVector TargetLocation = FVector(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FQuat TargetRotation = FQuat(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FVector LockLocation = FVector(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FQuat LockRotation = FQuat(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FVector LockComponentRelativeLocation = FVector(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FQuat LockComponentRelativeRotation = FQuat(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FVector LockMovementBaseRelativeLocation = FVector(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FQuat LockMovementBaseRelativeRotation = FQuat(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FVector OffsetTargetLocation = FVector(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FQuat OffsetTargetRotation = FQuat(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FSpringVectorState OffsetSpringState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FVector OffsetLocation = FVector(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FQuat OffsetRotation = FQuat(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FVector IkLocation = FVector(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FQuat IkRotation = FQuat(ForceInit);
};

USTRUCT(BlueprintType)
struct BECORE_API FFeetState
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = -1, ClampMax = 1))
	float FootPlantedAmount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ClampMax = 1))
	float FeetCrossingAmount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FFootState Left;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FFootState Right;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FVector2D MinMaxPelvisOffsetZ = FVector2D(ForceInit);
};

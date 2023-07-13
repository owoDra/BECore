// Copyright owoDra

#pragma once

#include "Utility/AlsMath.h"
#include "BEFeetState.generated.h"

USTRUCT(BlueprintType)
struct BECORE_API FAlsFootState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ClampMax = 1))
	float IkAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ClampMax = 1))
	float LockAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FVector TargetLocation{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FQuat TargetRotation{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FVector LockLocation{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FQuat LockRotation{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FVector LockComponentRelativeLocation{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FQuat LockComponentRelativeRotation{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FVector LockMovementBaseRelativeLocation{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FQuat LockMovementBaseRelativeRotation{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FVector OffsetTargetLocation{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FQuat OffsetTargetRotation{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FAlsSpringVectorState OffsetSpringState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FVector OffsetLocation{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FQuat OffsetRotation{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FVector IkLocation{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FQuat IkRotation{ForceInit};
};

USTRUCT(BlueprintType)
struct BECORE_API FBEFeetState
{
	GENERATED_BODY()

	// Choose whether a foot is planted or about to plant when stopping using the foot planted animation
	// curve. A value less than 0.5 means the foot is planted and a value more than 0.5 means the
	// foot is still in the air. The foot planted curve also determines which foot is planted (or
	// about to plant). Positive values mean the right foot is planted, negative values mean the left.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = -1, ClampMax = 1))
	float FootPlantedAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ClampMax = 1))
	float FeetCrossingAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FAlsFootState Left;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FAlsFootState Right;

	// Pelvis

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FVector2D MinMaxPelvisOffsetZ{ForceInit};
};

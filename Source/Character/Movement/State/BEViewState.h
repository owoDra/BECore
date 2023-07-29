// Copyright owoDra

#pragma once

#include "BEViewState.generated.h"

USTRUCT(BlueprintType)
struct BECORE_API FViewNetworkSmoothingState
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	bool bEnabled = false;

	// Used to track the time of the last server move.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ForceUnits = "s"))
	float ServerTime = 0.0f;

	// Used to track client time as we try to match the server.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ForceUnits = "s"))
	float ClientTime = 0.0f;

	// Used for remembering how much time passed between server corrections.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ForceUnits = "s"))
	float Duration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FRotator InitialRotation = FRotator(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FRotator Rotation = FRotator(ForceInit);
};

USTRUCT(BlueprintType)
struct BECORE_API FViewState
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FViewNetworkSmoothingState NetworkSmoothing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FRotator Rotation = FRotator(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ForceUnits = "deg/s"))
	float YawSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float PreviousYawAngle = 0.0f;
};

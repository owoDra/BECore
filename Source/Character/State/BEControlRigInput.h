// Copyright owoDra

#pragma once

#include "BEControlRigInput.generated.h"

USTRUCT(BlueprintType)
struct BECORE_API FControlRigInput
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	bool bUseHandIkBones{false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	bool bUseFootIkBones{false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ClampMax = 1))
	float VelocityBlendForwardAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ClampMax = 1))
	float VelocityBlendBackwardAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float SpineYawAngle{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FQuat FootLeftIkRotation{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FVector FootLeftIkLocation{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ClampMax = 1))
	float FootLeftIkAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FQuat FootRightIkRotation{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FVector FootRightIkLocation{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ClampMax = 1))
	float FootRightIkAmount{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FVector2D MinMaxPelvisOffsetZ{ForceInit};
};

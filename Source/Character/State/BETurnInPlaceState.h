// Copyright owoDra

#pragma once

#include "BETurnInPlaceState.generated.h"

class UBETurnInPlaceSettings;

USTRUCT(BlueprintType)
struct BECORE_API FTurnInPlaceState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ForceUnits = "s"))
	float ActivationDelay{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	TObjectPtr<UBETurnInPlaceSettings> QueuedSettings{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FName QueuedSlotName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ClampMax = 180, ForceUnits = "deg"))
	float QueuedTurnYawAngle{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (ClampMin = 0, ForceUnits = "x"))
	float PlayRate{1.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	bool bFootLockDisabled{false};
};

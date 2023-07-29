// Copyright owoDra

#pragma once

#include "BESpringState.generated.h"

USTRUCT(BlueprintType)
struct BECORE_API FSpringFloatState
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	float Velocity = ForceInit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	float PreviousTarget = ForceInit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	bool bStateValid = false;

public:
	void Reset()
	{
		Velocity = 0.f;
		PreviousTarget = 0.f;
		bStateValid = false;
	}
};

USTRUCT(BlueprintType)
struct BECORE_API FSpringVectorState
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FVector Velocity = FVector(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	FVector PreviousTarget = FVector(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE")
	bool bStateValid = false;

public:
	void Reset()
	{
		Velocity = FVector::ZeroVector;
		PreviousTarget = FVector::ZeroVector;
		bStateValid = false;
	}
};

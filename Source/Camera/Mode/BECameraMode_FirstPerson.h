// Copyright owoDra

#pragma once

#include "BECameraMode.h"

#include "BECameraMode_FirstPerson.generated.h"


/**
 * UBECameraMode_FirstPerson
 *
 *	FPP 視点用の CameraMode の基本クラス
 */
UCLASS(Abstract, Blueprintable)
class UBECameraMode_FirstPerson : public UBECameraMode
{
	GENERATED_BODY()
public:
	UBECameraMode_FirstPerson() {}

protected:
	virtual void PostActivateMode() override;
	virtual void PreDeactivateMode() override;


protected:
	virtual void UpdateView(float DeltaTime) override;
	void UpdateForTarget(float DeltaTime);

	void SetTargetCrouchOffset(FVector NewTargetOffset);
	void UpdateCrouchOffset(float DeltaTime);

	// しゃがみ時の遷移速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "First Person")
	float CrouchOffsetBlendMultiplier = 8.0f;

	float CrouchOffsetBlendPct = 1.0f;
	FVector InitialCrouchOffset = FVector::ZeroVector;
	FVector TargetCrouchOffset = FVector::ZeroVector;
	FVector CurrentCrouchOffset = FVector::ZeroVector;
};

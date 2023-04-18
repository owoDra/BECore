// Copyright Eigi Chin

#pragma once

#include "CoreMinimal.h"
#include "BECameraMode.h"
#include "Curves/CurveFloat.h"
#include "BECameraMode_FirstPerson.generated.h"

class UCurveVector;


// =================================
//  クラス
// =================================

/**
 * UBECameraMode_FirstPerson
 *
 *	A basic first person camera mode.
 */
UCLASS(Abstract, Blueprintable)
class UBECameraMode_FirstPerson : public UBECameraMode
{
	GENERATED_BODY()

	// =================================
	//  初期化
	// =================================
public:
	UBECameraMode_FirstPerson();

	// Called when this camera mode is activated on the camera mode stack.
	virtual void OnActivation() override;

	// Called when this camera mode is deactivated on the camera mode stack.
	virtual void OnDeactivation() override;


	// =================================
	//  カメラ
	// =================================
protected:
	virtual void UpdateView(float DeltaTime) override;

	void UpdateForTarget(float DeltaTime);

	// Alters the speed that a crouch offset is blended in or out
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "First Person")
	float CrouchOffsetBlendMultiplier = 8.0f;
	
	void SetTargetCrouchOffset(FVector NewTargetOffset);
	void UpdateCrouchOffset(float DeltaTime);

	FVector InitialCrouchOffset = FVector::ZeroVector;
	FVector TargetCrouchOffset = FVector::ZeroVector;
	float CrouchOffsetBlendPct = 1.0f;
	FVector CurrentCrouchOffset = FVector::ZeroVector;


	// =================================
	//  FPP Mask
	// =================================
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPP Mask")
		FName FPPMaskParamName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPP Mask")
		TSet<int> FPPMaterialIndexes;
};

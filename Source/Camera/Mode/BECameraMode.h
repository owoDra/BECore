// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "UObject/Object.h"

#include "Containers/Array.h"
#include "Engine/World.h"
#include "GameplayTagContainer.h"
#include "Math/Rotator.h"
#include "Math/UnrealMathSSE.h"
#include "Math/Vector.h"
#include "Templates/SubclassOf.h"
#include "UObject/UObjectGlobals.h"

#include "BECameraMode.generated.h"

class UBECameraComponent;
class AActor;
class UCanvas;


// =================================
//  列挙型
// =================================

/**
 * EBECameraModeBlendFunction
 *
 *	Blend function used for transitioning between camera modes.
 */
UENUM(BlueprintType)
enum class EBECameraModeBlendFunction : uint8
{
	// Does a simple linear interpolation.
	Linear,

	// Immediately accelerates, but smoothly decelerates into the target.  Ease amount controlled by the exponent.
	EaseIn,

	// Smoothly accelerates, but does not decelerate into the target.  Ease amount controlled by the exponent.
	EaseOut,

	// Smoothly accelerates and decelerates.  Ease amount controlled by the exponent.
	EaseInOut,

	COUNT	UMETA(Hidden)
};


// =================================
//  構造体
// =================================

/**
 * FBECameraModeView
 *
 *	View data produced by the camera mode that is used to blend camera modes.
 */
struct FBECameraModeView
{
public:

	FBECameraModeView();

	void Blend(const FBECameraModeView& Other, float OtherWeight);

public:

	FVector Location;
	FRotator Rotation;
	FRotator ControlRotation;
	float FieldOfView;
};


// =================================
//  クラス
// =================================

/**
 * UBECameraMode
 *
 *	Base class for all camera modes.
 */
UCLASS(Abstract, NotBlueprintable)
class BECORE_API UBECameraMode : public UObject
{
	GENERATED_BODY()

	// =================================
	//  初期化
	// =================================
public:
	UBECameraMode();

	// Called when this camera mode is activated on the camera mode stack.
	virtual void OnActivation() {};

	// Called when this camera mode is deactivated on the camera mode stack.
	virtual void OnDeactivation() {};


	// =================================
	//  ユーティリティ
	// =================================
public:
	UBECameraComponent* GetBECameraComponent() const;

	virtual UWorld* GetWorld() const override;

	AActor* GetTargetActor() const;

	const FBECameraModeView& GetCameraModeView() const { return View; }

	virtual void DrawDebug(UCanvas* Canvas) const;


	// =================================
	//  カメラ
	// =================================
public:
	void UpdateCameraMode(float DeltaTime);

	float GetBlendTime() const { return BlendTime; }
	float GetBlendWeight() const { return BlendWeight; }
	void SetBlendWeight(float Weight);

	FGameplayTag GetCameraTypeTag() const
	{
		return CameraTypeTag;
	}

protected:
	virtual FVector GetPivotLocation() const;
	virtual FRotator GetPivotRotation() const;

	virtual void UpdateView(float DeltaTime);
	virtual void UpdateBlending(float DeltaTime);

	// A tag that can be queried by gameplay code that cares when a kind of camera mode is active
	// without having to ask about a specific mode (e.g., when aiming downsights to get more accuracy)
	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	FGameplayTag CameraTypeTag;

	// View output produced by the camera mode.
	FBECameraModeView View;

	// The horizontal field of view (in degrees).
	UPROPERTY(EditDefaultsOnly, Category = "View", Meta = (UIMin = "5.0", UIMax = "170", ClampMin = "5.0", ClampMax = "170.0"))
	float FieldOfView;

	// Minimum view pitch (in degrees).
	UPROPERTY(EditDefaultsOnly, Category = "View", Meta = (UIMin = "-89.9", UIMax = "89.9", ClampMin = "-89.9", ClampMax = "89.9"))
	float ViewPitchMin;

	// Maximum view pitch (in degrees).
	UPROPERTY(EditDefaultsOnly, Category = "View", Meta = (UIMin = "-89.9", UIMax = "89.9", ClampMin = "-89.9", ClampMax = "89.9"))
	float ViewPitchMax;

	// How long it takes to blend in this mode.
	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	float BlendTime;

	// Function used for blending.
	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	EBECameraModeBlendFunction BlendFunction;

	// Exponent used by blend functions to control the shape of the curve.
	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	float BlendExponent;

	// Linear blend alpha used to determine the blend weight.
	float BlendAlpha;

	// Blend weight calculated using the blend alpha and function.
	float BlendWeight;

protected:
	/** If true, skips all interpolation and puts camera in ideal location.  Automatically set to false next frame. */
	UPROPERTY(transient)
	uint32 bResetInterpolation:1;
};


/**
 * UBECameraModeStack
 *
 *	Stack used for blending camera modes.
 */
UCLASS()
class UBECameraModeStack : public UObject
{
	GENERATED_BODY()

public:

	UBECameraModeStack();

	void ActivateStack();
	void DeactivateStack();

	bool IsStackActivate() const { return bIsActive; }

	void PushCameraMode(TSubclassOf<UBECameraMode> CameraModeClass);

	bool EvaluateStack(float DeltaTime, FBECameraModeView& OutCameraModeView);

	void DrawDebug(UCanvas* Canvas) const;

	// Gets the tag associated with the top layer and the blend weight of it
	void GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const;

protected:

	UBECameraMode* GetCameraModeInstance(TSubclassOf<UBECameraMode> CameraModeClass);

	void UpdateStack(float DeltaTime);
	void BlendStack(FBECameraModeView& OutCameraModeView) const;

protected:

	bool bIsActive;

	UPROPERTY()
	TArray<UBECameraMode*> CameraModeInstances;

	UPROPERTY()
	TArray<UBECameraMode*> CameraModeStack;
};

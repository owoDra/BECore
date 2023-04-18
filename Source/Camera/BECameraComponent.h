// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "Camera/CameraComponent.h"

#include "Delegates/Delegate.h"
#include "UObject/UObjectGlobals.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"

#include "BECameraComponent.generated.h"

class UBECameraMode;
class UBECameraModeStack;
class UCanvas;
class UObject;
struct FGameplayTag;
struct FFrame;
struct FMinimalViewInfo;
template <class TClass> class TSubclassOf;

DECLARE_DELEGATE_RetVal(TSubclassOf<UBECameraMode>, FBECameraModeDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBECameraModeChangedSignature, float, Weight, FGameplayTag, Tag);


/**
 * UBECameraComponent
 *
 *	このプロジェクトにおける基本の Camera Component
 */
UCLASS()
class UBECameraComponent : public UCameraComponent
{
	GENERATED_BODY()

	//======================================
	//	初期化
	//======================================
public:
	UBECameraComponent(const FObjectInitializer& ObjectInitializer);

	//======================================
	//	ユーティリティ
	//======================================
public:
	// Returns the camera component if one exists on the specified actor.
	UFUNCTION(BlueprintPure, Category = "Camera")
	static UBECameraComponent* FindCameraComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UBECameraComponent>() : nullptr); }

	// Returns the target actor that the camera is looking at.
	virtual AActor* GetTargetActor() const { return GetOwner(); }


	//======================================
	//	カメラ
	//======================================
public:
	// Gets the tag associated with the top layer and the blend weight of it
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const;

	virtual void DrawDebug(UCanvas* Canvas) const;

	UPROPERTY(BlueprintAssignable)
	FBECameraModeChangedSignature OnCameraModeChanged;

	// Delegate used to query for the best camera mode.
	FBECameraModeDelegate DetermineCameraModeDelegate;

protected:
	virtual void OnRegister() override;
	virtual void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView) override;

	virtual void UpdateCameraModes();

	// Stack used to blend the camera modes.
	UPROPERTY()
	TObjectPtr<UBECameraModeStack> CameraModeStack;

private:
	FRotator PrevControlRotation;


	//======================================
	//	リコイル
	//======================================
public:
	// リコイルを追加
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void AddRecoilOffset(FVector2D Offset, float RecoveryDelay);
	
	UFUNCTION()
	void HandleRecoilRecovery();
	
protected:
	virtual FRotator UpdateRecoilOffset(float DeltaTime, FRotator DeltaRotation);

private:
	// 画面の回転に適用するリコイルオフセット
	FRotator RecoilOffset;

	// リコイルオフセットの目標値
	FRotator TargetRecoilOffset;

	FTimerHandle TimerRecoilRecovery;

	bool CanRecoilRecovery = false;


	//======================================
	//	ズーム
	//======================================
public:
	// ズームをする
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetZoom(float ZoomAmount, float InterpSpeed);

protected:
	virtual void UpdateZoomAmount(float DeltaTime);

private:
	// FOVのズーム量
	float CurrentZoomAmount = 1.0;

	// FOVのズーム目標量
	float TargetZoomAmount = 1.0;

	float ZoomInterpSpeed;

	bool isZooming;
};

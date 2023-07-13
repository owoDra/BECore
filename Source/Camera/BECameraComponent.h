// Copyright owoDra

#pragma once

#include "Camera/CameraComponent.h"

#include "Mode/BECameraMode.h"

#include "Templates/SubclassOf.h"
#include "UObject/UObjectGlobals.h"
#include "GameplayTagContainer.h"

#include "BECameraComponent.generated.h"

class UBECameraMode;
class UBECameraModeStack;
class APlayerController;
class APawn;
class UObject;
struct FGameplayTag;


/**
 * EBECameraRecoilState
 *
 *	カメラに加えられている Recoil 更新状態
 */
UENUM(BlueprintType)
enum class EBECameraRecoilState : uint8
{
	None,		// Recoil が適応されていない
	RecoilUp,	// Recoil による回転が適応中
	Recovery	// Recoil による回転から復元中
};


/**
 * UBECameraComponent
 *
 *	プレイヤーが視点操作可能にするための処理を行う
 */
UCLASS()
class BECORE_API UBECameraComponent : public UCameraComponent
{
	GENERATED_BODY()

public:
	UBECameraComponent(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void OnRegister() override;
	virtual void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView) override;

protected:
	UPROPERTY()
	TObjectPtr<UBECameraModeStack> CameraModeStack;

	FRotator PreviousControlRotation;
	FRotator ControlRotationDelta;

protected:
	/**
	 * ComputeCameraView
	 *
	 *	Camera の最終的な視点情報を計算する
	 */
	virtual void ComputeCameraView(float DeltaTime, FMinimalViewInfo& DesiredView);

	/**
	 * ComputeRecoilAmount
	 *
	 *	視点の出力データに適応するリコイルによる回転を計算し、その値を返す
	 */
	virtual void ComputeRecoilAmount(float DeltaTime, FBECameraModeView& CameraModeView);

	/**
	 * ComputeZoomAmount
	 *
	 *	視点の出力データに適応するFOVの倍率を計算し、その値を返す
	 */
	virtual void ComputeZoomAmount(float DeltaTime, FBECameraModeView& CameraModeView);


public:
	/**
	 * AddRecoilOffset
	 *
	 *	Camera にリコイルを追加する
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void AddRecoilOffset(FVector2D Offset, float RecoveryDelay);

protected:
	UFUNCTION()
	void HandleRecoilRecovery();

	// これから適応するリコイル量
	FRotator RecoilAmountToAdd;

	// 実際に適応されたリコイル量
	FRotator CurrentRecoilAmount;

	// 現在のリコイルの更新状態
	EBECameraRecoilState RecoilState;

	FTimerHandle TimerRecoilRecovery;


public:
	/**
	 * AddRecoilOffset
	 *
	 *	Camera にズームを適応する
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetZoom(float ZoomAmount, float InterpSpeed);

protected:
	// FOVのズーム量
	float CurrentZoomAmount = 1.0;

	// FOVのズーム目標量
	float TargetZoomAmount = 1.0;

	float ZoomInterpSpeed;

	bool isZooming;


public:
	APlayerController* GetOwnerController() const;

	UFUNCTION(BlueprintPure, Category = "Camera")
	static UBECameraComponent* FindCameraComponent(const APawn* Pawn);

	/**
	 * GetBlendInfo
	 *
	 *	現在の Stack の Blend 情報を取得する
	 */
	UFUNCTION(BlueprintPure, Category = "Camera")
	void GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const;

	/**
	 * PushCameraMode
	 *
	 *	Stack の最初に新しく CameraMode を追加し Blend を始める
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void PushCameraMode(TSubclassOf<UBECameraMode> CameraModeClass);

	/**
	 * GetControlRotationDelta
	 *
	 *	キャッシュした直前のフレームとの ControlRotation の差を返す
	 */
	UFUNCTION(BlueprintPure, Category = "Camera")
	FRotator GetControlRotationDelta() const { return ControlRotationDelta; }
};

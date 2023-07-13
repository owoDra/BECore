// Copyright owoDra

#pragma once

#include "UObject/Object.h"

#include "Containers/Array.h"
#include "Engine/World.h"
#include "GameplayTagContainer.h"
#include "Math/Rotator.h"
#include "Math/UnrealMathSSE.h"
#include "Math/Vector.h"
#include "Templates/SubclassOf.h"
#include "UObject/ObjectPtr.h"
#include "UObject/UObjectGlobals.h"

#include "BECameraMode.generated.h"

class UBECameraComponent;
class AActor;
class UCanvas;


/**
 * EBECameraModeBlendFunction
 *
 *	CameraMode 間の遷移に使用されるブレンド関数
 */
UENUM(BlueprintType)
enum class EBECameraModeBlendFunction : uint8
{
	Linear,
	EaseIn,
	EaseOut,
	EaseInOut,

	COUNT	UMETA(Hidden)
};


/**
 * EBECameraModeActivationState
 *
 *	CameraMode が有効化状態
 */
UENUM(BlueprintType)
enum class EBECameraModeActivationState : uint8
{
	PreActivate,	// アクティブになる前のブレンド中の状態
	Activated,		// アクティブな状態
	PreDeactevate,	// 非アクティブになる前のブレンド中の状態
	Deactevated,	// 非アクティブな状態

	COUNT	UMETA(Hidden)
};


/**
 * FBECameraModeView
 *
 *	CameraMode のブレンドに使用される CameraMode によって生成されたデータ
 */
struct FBECameraModeView
{
	FBECameraModeView();

public:
	void Blend(const FBECameraModeView& Other, float OtherWeight);

	FVector Location;
	FRotator Rotation;
	FRotator ControlRotation;
	float FieldOfView;
};


/**
 * UBECameraMode
 *
 *	カメラの仕様を決めるための CameraMode の基本クラス
 */
UCLASS(Abstract, NotBlueprintable)
class BECORE_API UBECameraMode : public UObject
{
	GENERATED_BODY()
public:
	UBECameraMode();
	
public:
	/**
	 * SetActivationState
	 *
	 *  CameraMode の ActivationState を変更する
	 */
	virtual void SetActivationState(EBECameraModeActivationState NewActivationState);

protected:
	/**
	 * PreActivateMode
	 *
	 *  CameraMode が Active になる前の Blend が開始する際に呼び出される
	 */
	virtual void PreActivateMode() {};

	/**
	 * PostActivateMode
	 *
	 *  CameraMode の Blend が終了し Active になった際に呼び出される
	 */
	virtual void PostActivateMode() {};

	/**
	 * PreDeactivateMode
	 *
	 *  CameraMode が Deactive になる前の Blend が開始する際に呼び出される
	 */
	virtual void PreDeactivateMode() {};

	/**
	 * PostDeactivateMode
	 *
	 *  CameraMode の Blend が終了し Deactive になった際に呼び出される
	 */
	virtual void PostDeactivateMode() {};

	virtual FVector GetPivotLocation() const;
	virtual FRotator GetPivotRotation() const;

	virtual void UpdateView(float DeltaTime);
	virtual void UpdateBlending(float DeltaTime);

	// CameraMode の最終的な出力データ
	FBECameraModeView View;

	// CameraMode を識別するための Tag
	// 例えば ADS 中かどうかを判断するときに便利です
	UPROPERTY(EditDefaultsOnly, Category = "Blending", meta = (Categories = "Camera.Type"))
	FGameplayTag CameraTypeTag;

	UPROPERTY(EditDefaultsOnly, Category = "View", Meta = (UIMin = "5.0", UIMax = "170", ClampMin = "5.0", ClampMax = "170.0"))
	float FieldOfView;

	UPROPERTY(EditDefaultsOnly, Category = "View", Meta = (UIMin = "-89.9", UIMax = "89.9", ClampMin = "-89.9", ClampMax = "89.9"))
	float ViewPitchMin;

	UPROPERTY(EditDefaultsOnly, Category = "View", Meta = (UIMin = "-89.9", UIMax = "89.9", ClampMin = "-89.9", ClampMax = "89.9"))
	float ViewPitchMax;

	// CameraMode の Blend にかかる時間
	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	float BlendTime;

	// CameraMode のブレンド関数
	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	EBECameraModeBlendFunction BlendFunction;

	// 曲線の形状を制御するためにブレンド関数で使用される指数
	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	float BlendExponent;

	// Blend の計算に用いる値
	float BlendAlpha;

	// CameraMode がどのくらい Blend されたかの値
	// 1.0 以上は Blend が完了されたということになる
	float BlendWeight;

	// CameraMode の有効化状態
	EBECameraModeActivationState ActivationState;

	// true の場合、すべての補間をスキップし、カメラを理想的な場所に配置します。次のフレームで自動的に false に設定されます
	UPROPERTY(transient)
	uint32 bResetInterpolation:1;

public:
	void UpdateCameraMode(float DeltaTime);

	void SetBlendWeight(float Weight);

	float GetBlendTime() const { return BlendTime; }
	float GetBlendWeight() const { return BlendWeight; }
	const FBECameraModeView& GetCameraModeView() const { return View; }
	FGameplayTag GetCameraTypeTag() const { return CameraTypeTag; }


public:
	UBECameraComponent* GetBECameraComponent() const;

	AActor* GetTargetActor() const;

	virtual UWorld* GetWorld() const override;
};


/**
 * UBECameraModeStack
 *
 *	CameraMode を Blend するため Stack
 */
UCLASS()
class UBECameraModeStack : public UObject
{
	GENERATED_BODY()
public:
	UBECameraModeStack();

protected:
	UBECameraMode* GetCameraModeInstance(TSubclassOf<UBECameraMode> CameraModeClass);

	/**
	 * UpdateStack
	 *
	 *  Stack 内の CameraMode の更新を行う。
	 *	ブレンドが完了した CameraMode を判定して Stack から除外する。
	 */
	void UpdateStack(float DeltaTime);

	/**
	 * BlendStack
	 *
	 *	Stack 内の CameraMode の Blend を行う
	 */
	void BlendStack(FBECameraModeView& OutCameraModeView) const;

public:
	/**
	 * PushCameraMode
	 *
	 *	Stack の最初に新しく CameraMode を追加し Blend を始める
	 */
	void PushCameraMode(TSubclassOf<UBECameraMode> CameraModeClass);

	/**
	 * EvaluateStack
	 *
	 *	CameraComponent から呼び出され Stack の更新と最終的な出力データを返す
	 */
	void EvaluateStack(float DeltaTime, FBECameraModeView& OutCameraModeView);

	/**
	 * GetBlendInfo
	 *
	 *	現在の Stack の Blend 情報を取得する
	 */
	void GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const;

protected:
	UPROPERTY()
	TArray<TObjectPtr<UBECameraMode>> CameraModeInstances;

	UPROPERTY()
	TArray<TObjectPtr<UBECameraMode>> CameraModeStack;
};

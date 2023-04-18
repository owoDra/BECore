// Copyright Eigi Chin

#pragma once

#include "CommonUserWidget.h"

#include "Settings/BESettingsShared.h"

#include "CoreMinimal.h"
#include "GameFramework/GameplayMessageSubsystem.h"

#include "BEReticleWidgetBase.generated.h"

class UBEWeaponInstance;
class UBEInventoryItemInstance;
class UBESettingsShared;
class UPanelWidget;
class UImage;
class UHitMarkerConfirmationWidget;
class UCircumferenceMarkerWidget;
enum class EReticleStyle : uint8;
struct FReticle;
struct FBEVerbMessage;
struct FGameplayTag;
struct FGameplayMessageListenerHandle;

//////////////////////////////////////////////////////

/**
 *  ReticleStyle が Auto の時にカスタムレティクルかデフォルトレティクルを使用するか決める
 */
UENUM(BlueprintType)
enum class EReticleChangePolicy : uint8
{
	AlwaysUseCustom,	// 常にカスタムレティクルを使用
	AlwaysUseDefault,	// 常にデフォルトレティクルを使用
	UseDefaultOnAim,	// エイム時にデフォルトレティクルを使用し、エイムしてないときはカスタムレティクルを使用
	UseCustomOnAim		// エイム時にカスタムレティクルを使用し、エイムしてないときはデフォルトレティクルを使用
};

/**
 *  現在表示しているレティクル
 */
UENUM(BlueprintType)
enum class EShowingReticle : uint8
{
	None,
	Default,
	Custom
};


//////////////////////////////////////////////////////

/**
* UBEReticleWidgetBase
* 
*  本プロジェクトのレティクルのベースウィジェット
*/
UCLASS(Abstract)
class UBEReticleWidgetBase : public UCommonUserWidget
{
	GENERATED_BODY()

	//======================================
	//	初期化
	//======================================
public:
	UBEReticleWidgetBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintCallable)
		virtual void InitializeFromWeapon(UBEWeaponInstance* InWeapon, UBEInventoryItemInstance* InItem);

protected:
	UFUNCTION(BlueprintImplementableEvent)
		void OnWeaponInitialized();

private:
	TObjectPtr<UBEWeaponInstance> WeaponInstance;

	TObjectPtr<UBEInventoryItemInstance> InventoryInstance;


	//======================================
	//	コンフィグ
	//======================================
public:
	// ReticleStyle が Custom でも ReticleChangePolicy に従う
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Reticle")
		bool bForceUseReticleChangePolicy = false;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Reticle")
		EReticleChangePolicy ReticleChangePolicy = EReticleChangePolicy::UseDefaultOnAim;


	//======================================
	//	イベント
	//======================================
protected:
	virtual void BindOrUnbind_SettingChanged(bool NewBind = true);

	virtual void BindOrUnbind_AimChanged(bool NewBind = true);

	virtual void BindOrUnbind_Elimination(bool NewBind = true);

private:
	UFUNCTION()
		virtual void HandleSettingChanged(UBESettingsShared* InSettings);

	UFUNCTION()
		virtual void HandleAimChanged(bool isAiming);

	UFUNCTION()
		virtual void HandleElimination(FGameplayTag Tag, const FBEVerbMessage& Message);

	FDelegateHandle SettingChangeDelegateHandle;

	FGameplayMessageListenerHandle EliminationMessageListenerHandle;

protected:
	UFUNCTION(BlueprintImplementableEvent)
		void OnSettingChanged();

	UFUNCTION(BlueprintImplementableEvent)
		void OnAimChanged(bool IsAiming);

	UFUNCTION(BlueprintImplementableEvent)
		void OnEliminateTarget();

	UFUNCTION(BlueprintImplementableEvent)
		void ToCustomReticle();

	UFUNCTION(BlueprintImplementableEvent)
		void ToDefaultReticle();


	//======================================
	//	状態
	//======================================
public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Reticle")
		FReticle GetCurrentReticle() const { return CurrentReticle; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Reticle")
		EReticleStyle GetReticleStyle() const { return ReticleStyle; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Reticle")
		FLinearColor GetAimingReticleColor() const { return (bOverrideAimingReticleColor) ? AimingReticleColorOverride : AimingReticleColor; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Reticle")
		bool ShouldShowHitMarker() const { return bShowHitMarker; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Reticle")
		bool ShouldShowEliminationMarker() const { return bShowEliminationMarker; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Reticle")
		bool ShouldApplyFiringError() const { return bApplyFiringError; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Reticle")
		bool IsAiming() const { return bAiming; }

protected:
	virtual void RefreshReticleStyle();

	virtual void SetShowingReticle(EShowingReticle NewShowReticle);

private:
	FReticle CurrentReticle;

	EReticleStyle ReticleStyle;

	FLinearColor AimingReticleColor;

	FLinearColor AimingReticleColorOverride;

	bool bOverrideAimingReticleColor = false;

	bool bShowHitMarker = true;

	bool bShowEliminationMarker = true;

	bool bApplyFiringError = true;

	bool bAiming = false;

	EShowingReticle ShowingReticle = EShowingReticle::None;


	//======================================
	//	武器拡散
	//======================================
public:
	/**
	 * 現在の武器の最大拡散半径をスクリーン空間単位 (ピクセル) で取得
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Reticle")
		virtual float GetMaxScreenspaceSpreadRadius() const;

	UFUNCTION(BlueprintCallable, Category = "Reticle")
		virtual void UpdateSpreadRadius(float InRadius);

protected:
	UFUNCTION(BlueprintImplementableEvent)
		void OnUpdateSpreadRadius(float InRadius);

protected:
	/**
	 * 現在の武器の拡散角度を取得
	 */
	virtual float GetWeaponSpreadAngle() const;


	//======================================
	//	ユーティリティ
	//======================================
public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Reticle")
		UBEWeaponInstance* GetWeaponInstance() const { return WeaponInstance; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Reticle")
		UBEInventoryItemInstance* GetInventoryInstance() const { return InventoryInstance; }


	//======================================
	//	バインドウィジェット
	//======================================
private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
		TObjectPtr<UPanelWidget> Panel_DefaultReticle;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
		TObjectPtr<UPanelWidget> Panel_CustomReticle;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
		TObjectPtr<UHitMarkerConfirmationWidget> HitMarker;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
		TObjectPtr<UImage> EliminationMarker;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
		TObjectPtr<UCircumferenceMarkerWidget> CustomReticle_Horizontal;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
		TObjectPtr<UCircumferenceMarkerWidget> CustomReticle_Vertical;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
		TObjectPtr<UImage> CustomReticle_Dot;
};

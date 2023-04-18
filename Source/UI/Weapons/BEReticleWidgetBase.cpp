// Copyright Eigi Chin

#include "BEReticleWidgetBase.h"

#include "Weapons/InventoryFragment_ReticleConfig.h"
#include "Weapons/BEWeaponInstance.h"
#include "Weapons/BERangedWeaponInstance.h"
#include "HitMarkerConfirmationWidget.h"
#include "CircumferenceMarkerWidget.h"
#include "Player/BELocalPlayer.h"
#include "Character/BECharacter.h"
#include "Messages/BEVerbMessage.h"
#include "BEGameplayTags.h"

#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "GameFramework/PlayerState.h"

#include "Inventory/BEInventoryItemInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEReticleWidgetBase)

//////////////////////////////////////////////

UBEReticleWidgetBase::UBEReticleWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UBEReticleWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	BindOrUnbind_SettingChanged(true);
	BindOrUnbind_AimChanged(true);
	BindOrUnbind_Elimination(true);
}

void UBEReticleWidgetBase::NativeDestruct()
{
	Super::NativeDestruct();

	BindOrUnbind_SettingChanged(false);
	BindOrUnbind_AimChanged(false);
	BindOrUnbind_Elimination(false);
}

void UBEReticleWidgetBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bApplyFiringError && !bAiming)
	{
		UpdateSpreadRadius(GetMaxScreenspaceSpreadRadius());
	}
}

void UBEReticleWidgetBase::InitializeFromWeapon(UBEWeaponInstance* InWeapon, UBEInventoryItemInstance* InItem)
{
	WeaponInstance = InWeapon;
	InventoryInstance = InItem;
	if (InventoryInstance)
	{
		if (const UInventoryFragment_ReticleConfig* ReticleConfig = InventoryInstance->FindFragmentByClass<UInventoryFragment_ReticleConfig>())
		{
			AimingReticleColor = ReticleConfig->AimingReticleColor;
		}
	}
	OnWeaponInitialized();
}


void UBEReticleWidgetBase::BindOrUnbind_SettingChanged(bool NewBind)
{
	if (UBELocalPlayer* BELP = Cast<UBELocalPlayer>(GetOwningLocalPlayer()))
	{
		if (UBESettingsShared* Settings = BELP->GetSharedSettings())
		{
			if (NewBind)
			{
				if (SettingChangeDelegateHandle.IsValid())
				{
					BindOrUnbind_SettingChanged(false);
				}

				SettingChangeDelegateHandle = Settings->OnSettingApplied.AddUObject(this, &ThisClass::HandleSettingChanged);
				HandleSettingChanged(Settings);
			}
			else
			{
				Settings->OnSettingChanged.Remove(SettingChangeDelegateHandle);
			}
		}
	}
}

void UBEReticleWidgetBase::BindOrUnbind_AimChanged(bool NewBind)
{
	if (ABECharacter* BEChara = Cast<ABECharacter>(GetOwningPlayerPawn()))
	{
		if (NewBind)
		{
			BEChara->OnAimChanged.AddDynamic(this, &ThisClass::HandleAimChanged);
			HandleAimChanged(BEChara->bIsAiming);
		}
		else
		{
			BEChara->OnAimChanged.RemoveDynamic(this, &ThisClass::HandleAimChanged);
		}
	}
}

void UBEReticleWidgetBase::BindOrUnbind_Elimination(bool NewBind)
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UGameplayMessageSubsystem* GMS = GI->GetSubsystem<UGameplayMessageSubsystem>())
		{
			if (NewBind)
			{
				if (EliminationMessageListenerHandle.IsValid())
				{
					BindOrUnbind_Elimination(false);
				}

				EliminationMessageListenerHandle = GMS->RegisterListener(TAG_Message_Elimination, this, &ThisClass::HandleElimination);
			}
			else
			{
				GMS->UnregisterListener(EliminationMessageListenerHandle);
			}
		}
	}
}

void UBEReticleWidgetBase::HandleSettingChanged(UBESettingsShared* InSettings)
{
	if (InSettings)
	{
		CurrentReticle = InSettings->GetCurrentReticle();
		ReticleStyle = InSettings->GetReticleStyle();
		bShowHitMarker = InSettings->GetShowHitMarker();
		bShowEliminationMarker = InSettings->GetShowEliminationMarker();
		bApplyFiringError = InSettings->GetApplyFiringError();
		bOverrideAimingReticleColor = InSettings->ShouldOverrideAimingReticleColor();
		AimingReticleColorOverride = InSettings->GetAimingReticleColor();

		/////////////////////////////////
		// インライン、アウトライン

		FSlateBrushOutlineSettings Outline;
		Outline.CornerRadii = FVector4(0.0, 0.0, 0.0, 0.0);
		Outline.Color = FSlateColor(CurrentReticle.OutlineColor);
		Outline.Width = CurrentReticle.OutlineThickness;
		Outline.RoundingType = ESlateBrushRoundingType::FixedRadius;
		Outline.bUseBrushTransparency = true;

		FSlateBrush HorizontalLineBrush;
		HorizontalLineBrush.DrawAs = ESlateBrushDrawType::RoundedBox;
		HorizontalLineBrush.Tiling = ESlateBrushTileType::NoTile;
		HorizontalLineBrush.ImageSize = FVector2D(CurrentReticle.LineThicknessHori, CurrentReticle.LineLengthHori) + FVector2D(CurrentReticle.OutlineThickness * 2);
		HorizontalLineBrush.TintColor = FSlateColor(CurrentReticle.InlineColor);
		HorizontalLineBrush.OutlineSettings = Outline;

		FSlateBrush VerticalLineBrush = HorizontalLineBrush;
		VerticalLineBrush.ImageSize = FVector2D(CurrentReticle.LineThicknessVert, CurrentReticle.LineLengthVert) + FVector2D(CurrentReticle.OutlineThickness * 2);

		CustomReticle_Horizontal->SetBrush(HorizontalLineBrush);
		CustomReticle_Vertical->SetBrush(VerticalLineBrush);


		/////////////////////////////////
		// ドット

		FSlateBrush DotBrush;
		DotBrush.DrawAs = ESlateBrushDrawType::RoundedBox;
		DotBrush.Tiling = ESlateBrushTileType::NoTile;
		DotBrush.ImageSize = FVector2D(CurrentReticle.DotSize) + FVector2D(CurrentReticle.OutlineThickness * 2);
		DotBrush.TintColor = FSlateColor(CurrentReticle.DotColor);
		DotBrush.OutlineSettings = Outline;

		CustomReticle_Dot->SetBrush(DotBrush);
		CustomReticle_Dot->SetVisibility((CurrentReticle.bShowCenterDot) ? (ESlateVisibility::Visible) : (ESlateVisibility::Collapsed));

		/////////////////////////////////
		// ヒットマーカー

		HitMarker->SetVisibility((bShowHitMarker) ? (ESlateVisibility::HitTestInvisible) : (ESlateVisibility::Collapsed));

		/////////////////////////////////
		// エリミネーションマーカー

		EliminationMarker->SetVisibility((bShowEliminationMarker) ? (ESlateVisibility::HitTestInvisible) : (ESlateVisibility::Collapsed));

		/////////////////////////////////
		// ファイアリングエラー

		if (!bApplyFiringError)
		{
			UpdateSpreadRadius(0.0);
		}
	}

	RefreshReticleStyle();

	OnSettingChanged();
}

void UBEReticleWidgetBase::HandleAimChanged(bool isAiming)
{
	bAiming = isAiming;

	if (bAiming)
	{
		UpdateSpreadRadius(0.0);
	}

	RefreshReticleStyle();

	OnAimChanged(isAiming);
}

void UBEReticleWidgetBase::HandleElimination(FGameplayTag Tag, const FBEVerbMessage& Message)
{
	if ((Message.Instigator != Message.Target) && (Cast<APlayerState>(Message.Instigator) == GetOwningPlayerState()))
	{
		OnEliminateTarget();
	}
}


void UBEReticleWidgetBase::RefreshReticleStyle()
{
	//////////////////////////////////////////
	// ReticleStyle が Default

	if (ReticleStyle == EReticleStyle::Default)
	{
		Panel_DefaultReticle->SetVisibility(ESlateVisibility::HitTestInvisible);
		Panel_CustomReticle->SetVisibility(ESlateVisibility::Collapsed);
		SetShowingReticle(EShowingReticle::Default);
		return;
	}


	//////////////////////////////////////////
	// ReticleStyle が Custom

	else if (ReticleStyle == EReticleStyle::Custom)
	{
		if (!bForceUseReticleChangePolicy)
		{
			Panel_DefaultReticle->SetVisibility(ESlateVisibility::Collapsed);
			Panel_CustomReticle->SetVisibility(ESlateVisibility::HitTestInvisible);
			SetShowingReticle(EShowingReticle::Custom);
			return;
		}
	}


	//////////////////////////////////////////
	// ReticleStyle が Auto
	{
		//////////////////////////////////////////
		// ReticleChangePolicy が AlwaysUseCustom

		if (ReticleChangePolicy == EReticleChangePolicy::AlwaysUseCustom)
		{
			Panel_DefaultReticle->SetVisibility(ESlateVisibility::Collapsed);
			Panel_CustomReticle->SetVisibility(ESlateVisibility::HitTestInvisible);
			SetShowingReticle(EShowingReticle::Custom);
			return;
		}


		//////////////////////////////////////////
		// ReticleChangePolicy が AlwaysUseDefault

		else if (ReticleChangePolicy == EReticleChangePolicy::AlwaysUseDefault)
		{
			Panel_DefaultReticle->SetVisibility(ESlateVisibility::HitTestInvisible);
			Panel_CustomReticle->SetVisibility(ESlateVisibility::Collapsed);
			SetShowingReticle(EShowingReticle::Default);
			return;
		}


		//////////////////////////////////////////
		// ReticleChangePolicy が UseCustomOnAim

		else if (ReticleChangePolicy == EReticleChangePolicy::UseCustomOnAim)
		{
			Panel_DefaultReticle->SetVisibility(ESlateVisibility::HitTestInvisible);
			Panel_CustomReticle->SetVisibility(ESlateVisibility::HitTestInvisible);

			if (bAiming)
			{
				SetShowingReticle(EShowingReticle::Custom);
			}
			else
			{
				SetShowingReticle(EShowingReticle::Default);
			}
		}


		//////////////////////////////////////////
		// ReticleChangePolicy が UseDefaultOnAim

		else
		{
			Panel_DefaultReticle->SetVisibility(ESlateVisibility::HitTestInvisible);
			Panel_CustomReticle->SetVisibility(ESlateVisibility::HitTestInvisible);

			if (bAiming)
			{
				SetShowingReticle(EShowingReticle::Default);
			}
			else
			{
				SetShowingReticle(EShowingReticle::Custom);
			}
		}
	}
}

void UBEReticleWidgetBase::SetShowingReticle(EShowingReticle NewShowReticle)
{
	if (ShowingReticle != NewShowReticle)
	{
		ShowingReticle = NewShowReticle;

		if (NewShowReticle == EShowingReticle::Default)
		{
			ToDefaultReticle();
		}
		else if (NewShowReticle == EShowingReticle::Custom)
		{
			ToCustomReticle();
		}
	}
}


float UBEReticleWidgetBase::GetMaxScreenspaceSpreadRadius() const
{
	const float LongShotDistance = 10000.f;

	APlayerController* PC = GetOwningPlayer();
	if (PC && PC->PlayerCameraManager)
	{
		// A weapon's spread can be thought of as a cone shape. To find the screenspace spread for reticle visualization,
		// we create a line on the edge of the cone at a long distance. The end of that point is on the edge of the cone's circle.
		// We then project it back onto the screen. Its distance from screen center is the spread radius.

		// This isn't perfect, due to there being some distance between the camera location and the gun muzzle.

		const float SpreadRadiusRads = FMath::DegreesToRadians(GetWeaponSpreadAngle() * 0.5f);
		const float SpreadRadiusAtDistance = FMath::Tan(SpreadRadiusRads) * LongShotDistance;

		FVector CamPos;
		FRotator CamOrient;
		PC->PlayerCameraManager->GetCameraViewPoint(CamPos, CamOrient);

		FVector CamForwDir = CamOrient.RotateVector(FVector::ForwardVector);
		FVector CamUpDir = CamOrient.RotateVector(FVector::UpVector);

		FVector OffsetTargetAtDistance = CamPos + (CamForwDir * LongShotDistance) + (CamUpDir * SpreadRadiusAtDistance);

		FVector2D OffsetTargetInScreenspace;

		if (PC->ProjectWorldLocationToScreen(OffsetTargetAtDistance, OffsetTargetInScreenspace, true))
		{
			int32 ViewportSizeX(0), ViewportSizeY(0);
			PC->GetViewportSize(ViewportSizeX, ViewportSizeY);

			const FVector2D ScreenSpaceCenter(FVector::FReal(ViewportSizeX) * 0.5f, FVector::FReal(ViewportSizeY) * 0.5f);

			return (OffsetTargetInScreenspace - ScreenSpaceCenter).Length();
		}
	}

	return 0.0f;
}

void UBEReticleWidgetBase::UpdateSpreadRadius(float InRadius)
{
	CustomReticle_Horizontal->SetRadius(InRadius + CurrentReticle.LineOffsetHori);
	CustomReticle_Vertical->SetRadius(InRadius + CurrentReticle.LineOffsetVert);

	OnUpdateSpreadRadius(InRadius);
}

float UBEReticleWidgetBase::GetWeaponSpreadAngle() const
{
	if (const UBERangedWeaponInstance* RangedWeapon = Cast<const UBERangedWeaponInstance>(WeaponInstance))
	{
		const float BaseSpreadAngle = RangedWeapon->GetCalculatedSpreadAngle();
		const float SpreadAngleMultiplier = RangedWeapon->GetCalculatedSpreadAngleMultiplier();
		const float ActualSpreadAngle = BaseSpreadAngle * SpreadAngleMultiplier;

		return ActualSpreadAngle;
	}
	else
	{
		return 0.0f;
	}
}

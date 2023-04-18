// Copyright Eigi Chin

#include "SCircumferenceMarkerWidget.h"
#include "Engine/UserInterfaceSettings.h"
#include "Kismet/GameplayStatics.h"


//======================================
//	初期化
//======================================

SLATE_IMPLEMENT_WIDGET(SCircumferenceMarkerWidget)
void SCircumferenceMarkerWidget::PrivateRegisterAttributes(FSlateAttributeInitializer& AttributeInitializer)
{
	SLATE_ADD_MEMBER_ATTRIBUTE_DEFINITION_WITH_NAME(AttributeInitializer, "MarkerBrush", MarkerBrush, EInvalidateWidgetReason::Layout);
	SLATE_ADD_MEMBER_ATTRIBUTE_DEFINITION_WITH_NAME(AttributeInitializer, "Radius", Radius, EInvalidateWidgetReason::Layout);
}

SCircumferenceMarkerWidget::SCircumferenceMarkerWidget()
	: MarkerBrush(*this)
	, Radius(*this)
{
	SetCanTick(false);
	bCanSupportFocus = false;
}

void SCircumferenceMarkerWidget::Construct(const FArguments& InArgs)
{
	MarkerList = InArgs._MarkerList;

	MarkerBrush.Assign(*this, InArgs._MarkerBrush);
	Radius.Assign(*this, InArgs._Radius);
}


//======================================
//	描画
//======================================

int32 SCircumferenceMarkerWidget::OnPaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled) const
{
	const bool bIsEnabled = ShouldBeEnabled(bParentEnabled);
	const ESlateDrawEffect DrawEffects = bIsEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;
	const FVector2D LocalCenter = AllottedGeometry.GetLocalPositionAtCoordinates(FVector2D(0.5f, 0.5f));
	const FSlateBrush* ImageBrush = MarkerBrush.Get();

	const bool bDrawMarkers = (MarkerList.Num() > 0) && (ImageBrush != nullptr);

	if (bDrawMarkers == true)
	{
		const FLinearColor MarkerColor = InWidgetStyle.GetColorAndOpacityTint() * ImageBrush->GetTint(InWidgetStyle);

		if (MarkerColor.A > KINDA_SMALL_NUMBER)
		{
			const float BaseRadius = *Radius.Get();
			const float ApplicationScale = GetDefault<UUserInterfaceSettings>()->ApplicationScale;
			for (FCircumferenceMarkerEntry Marker : MarkerList)
			{
				const FSlateRenderTransform MarkerTransform = GetMarkerRenderTransform(Marker, BaseRadius, ApplicationScale);

				const FVector2D ImageSize = ImageBrush->ImageSize * Marker.ImageScale;

				const FPaintGeometry Geometry(AllottedGeometry.ToPaintGeometry(ImageSize, FSlateLayoutTransform(LocalCenter - (ImageSize * 0.5f)), MarkerTransform, FVector2D(0.0f, 0.0f)));
				FSlateDrawElement::MakeBox(OutDrawElements, LayerId, Geometry, ImageBrush, DrawEffects, MarkerColor);
			}
		}
	}

	return LayerId;
}

FVector2D SCircumferenceMarkerWidget::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	const FSlateBrush* ImageBrush = MarkerBrush.Get();
	check(ImageBrush);
	const float SampledRadius = *Radius.Get();
	return FVector2D((ImageBrush->ImageSize.X + SampledRadius) * 2.0f, (ImageBrush->ImageSize.Y + SampledRadius) * 2.0f);
}

FSlateRenderTransform SCircumferenceMarkerWidget::GetMarkerRenderTransform(const FCircumferenceMarkerEntry& Marker, const float BaseRadius, const float HUDScale) const
{
	const FSlateBrush* ImageBrush = MarkerBrush.Get();
	const FVector2D ImageSize = ImageBrush->ImageSize * Marker.ImageScale;

	// Determine the radius to use for the corners
	float XRadius = BaseRadius + ImageSize.X * 0.5f;
	float YRadius = BaseRadius + ImageSize.X * 0.5f;

	// Get the angle and orientation for this reticle corner
	const float LocalRotationRadians = FMath::DegreesToRadians(Marker.ImageRotationAngle);
	const float PositionAngleRadians = FMath::DegreesToRadians(Marker.PositionAngle);

	// First rotate the corner image about the origin
	FSlateRenderTransform RotateAboutOrigin(Concatenate(FVector2D(-ImageSize.X * 0.5f, -ImageSize.Y * 0.5f), FQuat2D(LocalRotationRadians), FVector2D(ImageSize.X * 0.5f, ImageSize.Y * 0.5f)));

	// Move the rotated image to the right place on the spread radius
	return TransformCast<FSlateRenderTransform>(Concatenate(RotateAboutOrigin, FVector2D(XRadius * FMath::Sin(PositionAngleRadians) * HUDScale, -YRadius * FMath::Cos(PositionAngleRadians) * HUDScale)));
}


//======================================
//	設定
//======================================

void SCircumferenceMarkerWidget::SetRadius(TAttribute<const float*> NewRadius)
{
	Radius.Assign(*this, MoveTemp(NewRadius));
	Invalidate(EInvalidateWidgetReason::Layout);
}

void SCircumferenceMarkerWidget::SetBrush(TAttribute<const FSlateBrush*> NewBrush)
{
	MarkerBrush.Assign(*this, MoveTemp(NewBrush));
	Invalidate(EInvalidateWidgetReason::Layout);
}

void SCircumferenceMarkerWidget::SetMarkerList(TArray<FCircumferenceMarkerEntry>& NewMarkerList)
{
	MarkerList = NewMarkerList;
}

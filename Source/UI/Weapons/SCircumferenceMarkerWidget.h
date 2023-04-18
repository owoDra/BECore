// Copyright Eigi Chin

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateBrush.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SLeafWidget.h"
#include "Styling/CoreStyle.h"
#include "SCircumferenceMarkerWidget.generated.h"


//======================================
//	構造体
//======================================

USTRUCT(BlueprintType)
struct FCircumferenceMarkerEntry
{
	GENERATED_BODY()

	// マーカーを配置するときの中心から見た角度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ForceUnits=deg))
	float PositionAngle = 0.0f;

	// マーカーの画像の角度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ForceUnits=deg))
	float ImageRotationAngle = 0.0f;

	// マーカーの画像の角度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ForceUnits = deg))
	FVector2D ImageScale = FVector2D(1.0f);
};


//======================================
//	クラス
//======================================

/**
* SCircumferenceMarkerWidget
*  
*  レティクルの描画をするためのスレートウィジェット
*/
class SCircumferenceMarkerWidget : public SLeafWidget
{
	//======================================
	//	スレート定義
	//======================================

	SLATE_DECLARE_WIDGET(SCircumferenceMarkerWidget, SLeafWidget)

	SLATE_BEGIN_ARGS(SCircumferenceMarkerWidget)
		: _MarkerBrush(FCoreStyle::Get().GetBrush("Throbber.CircleChunk"))
	{
	}
		/** At which angles should a marker be drawn */
		SLATE_ARGUMENT(TArray<FCircumferenceMarkerEntry>, MarkerList)
		/** What each marker on the circumference looks like */
		SLATE_ATTRIBUTE(const FSlateBrush*, MarkerBrush)
		/** The radius of the circle */
		SLATE_ATTRIBUTE(const float*, Radius)
	SLATE_END_ARGS()


	//======================================
	//	初期化
	//======================================
public:
	SCircumferenceMarkerWidget();

	void Construct(const FArguments& InArgs);


	//======================================
	//	描画
	//======================================
public:
	virtual int32 OnPaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override;

	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
	virtual bool ComputeVolatility() const override { return true; }

private:
	FSlateRenderTransform GetMarkerRenderTransform(const FCircumferenceMarkerEntry& Marker, const float BaseRadius, const float HUDScale) const;


	//======================================
	//	設定
	//======================================
public:
	void SetRadius(TAttribute<const float*> NewRadius);
	void SetBrush(TAttribute<const FSlateBrush*> NewBrush);
	void SetMarkerList(TArray<FCircumferenceMarkerEntry>& NewMarkerList);

private:
	/** Angles around the reticle center to place ReticleCornerImage icons */
	TArray<FCircumferenceMarkerEntry> MarkerList;

	/** What each marker on the circumference looks like */
	TSlateAttribute<const FSlateBrush*> MarkerBrush;

	/** The radius of the circle */
	TSlateAttribute<const float*> Radius;
};

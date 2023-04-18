// Copyright Eigi Chin

#pragma once

#include "CoreMinimal.h"
#include "SlateFwd.h"
#include "UObject/ObjectMacros.h"
#include "Styling/SlateBrush.h"
#include "Widgets/SWidget.h"
#include "Components/Widget.h"
#include "SCircumferenceMarkerWidget.h"

#include "CircumferenceMarkerWidget.generated.h"


//======================================
//	クラス
//======================================

/**
* UCircumferenceMarkerWidget
*
*  レティクルの描画をするためのウィジェット
*/
UCLASS()
class UCircumferenceMarkerWidget : public UWidget
{
	GENERATED_BODY()

	//======================================
	//	初期化
	//======================================
public:
	UCircumferenceMarkerWidget(const FObjectInitializer& ObjectInitializer);

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;


	//======================================
	//	プロパティ
	//======================================
public:
	UFUNCTION(BlueprintCallable, Category = "Appearance")
		UMaterialInstanceDynamic* GetDynamicMaterial();

	virtual void SynchronizeProperties() override;

public:
	/** The list of positions/orientations to draw the markers at. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
	TArray<FCircumferenceMarkerEntry> MarkerList;

	/** The radius of the circle. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
	float Radius = 48.0f;

	/** The marker image to place around the circle. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
	FSlateBrush MarkerBrush;

	/** Whether reticle corner images are placed outside the spread radius */
	//@TODO: Make this a 0-1 float alignment instead (e.g., inside/on/outside the radius)?
	UPROPERTY(EditAnywhere, Category=Corner)
	uint8 bReticleCornerOutsideSpreadRadius : 1;

public:
	/** Sets the radius of the circle. */
	UFUNCTION(BlueprintCallable, Category = "Appearance")
		void SetRadius(float InRadius);

	/** Sets the brush of the marker. */
	UFUNCTION(BlueprintCallable, Category = "Appearance")
		void SetBrush(const FSlateBrush& InBrush);

private:
	/** Internal slate widget representing the actual marker visuals */
	TSharedPtr<SCircumferenceMarkerWidget> MyMarkerWidget;
};

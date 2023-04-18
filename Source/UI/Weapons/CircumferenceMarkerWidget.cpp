// Copyright Eigi Chin

#include "CircumferenceMarkerWidget.h"
#include "SCircumferenceMarkerWidget.h"


//======================================
//	初期化
//======================================

UCircumferenceMarkerWidget::UCircumferenceMarkerWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetVisibility(ESlateVisibility::HitTestInvisible);
	bIsVolatile = true;
}

void UCircumferenceMarkerWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	MyMarkerWidget.Reset();
}

TSharedRef<SWidget> UCircumferenceMarkerWidget::RebuildWidget()
{
	MyMarkerWidget = SNew(SCircumferenceMarkerWidget)
		.MarkerBrush(&MarkerBrush)
		.Radius(&Radius)
		.MarkerList(this->MarkerList);

	return MyMarkerWidget.ToSharedRef();
}


//======================================
//	プロパティ
//======================================

void UCircumferenceMarkerWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	if (MyMarkerWidget.IsValid())
	{
		MyMarkerWidget->SetRadius(&Radius);
		MyMarkerWidget->SetBrush(&MarkerBrush);
		MyMarkerWidget->SetMarkerList(MarkerList);
	}
}

UMaterialInstanceDynamic* UCircumferenceMarkerWidget::GetDynamicMaterial()
{
	UObject* Resource = MarkerBrush.GetResourceObject();
	if (UMaterialInterface* Material = Cast<UMaterialInterface>(Resource))
	{
		UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(Material);
		if (!DynamicMaterial)
		{
			DynamicMaterial = UMaterialInstanceDynamic::Create(Material, this);
			MarkerBrush.SetResourceObject(DynamicMaterial);

			if (MyMarkerWidget.IsValid())
			{
				MyMarkerWidget->SetBrush(&MarkerBrush);
			}
		}

		return DynamicMaterial;
	}

	return nullptr;
}

void UCircumferenceMarkerWidget::SetRadius(float InRadius)
{
	if (Radius != InRadius)
	{
		Radius = InRadius;

		if (MyMarkerWidget.IsValid())
		{
			MyMarkerWidget->SetRadius(&Radius);
		}
	}
}

void UCircumferenceMarkerWidget::SetBrush(const FSlateBrush& InBrush)
{
	if (MarkerBrush != InBrush)
	{
		MarkerBrush = InBrush;

		if (MyMarkerWidget.IsValid())
		{
			MyMarkerWidget->SetBrush(&MarkerBrush);
		}
	}
}

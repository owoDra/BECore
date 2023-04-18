// Copyright Eigi Chin

#include "BETabButtonBase.h"
#include "CommonLazyImage.h"
#include "CommonTextBlock.h"

void UBETabButtonBase::SetIconFromLazyObject(TSoftObjectPtr<UObject> LazyObject)
{
	if (LazyImage_Icon)
	{
		LazyImage_Icon->SetBrushFromLazyDisplayAsset(LazyObject);
	}
}

void UBETabButtonBase::SetIconBrush(const FSlateBrush& Brush)
{
	if (LazyImage_Icon)
	{
		LazyImage_Icon->SetBrush(Brush);
	}
}

void UBETabButtonBase::SetTabLabelInfo_Implementation(const FBETabDescriptor& TabLabelInfo)
{
	SetButtonText(TabLabelInfo.TabText);
	SetIconBrush(TabLabelInfo.IconBrush);
}

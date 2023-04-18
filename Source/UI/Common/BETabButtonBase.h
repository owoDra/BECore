// Copyright Eigi Chin

#pragma once

#include "CommonButtonBase.h"
#include "BETabListWidgetBase.h"
#include "UI/Foundation/BEButtonBase.h"

#include "BETabButtonBase.generated.h"

class UCommonLazyImage;
class UCommonTextBlock;

UCLASS(Abstract, Blueprintable, meta = (DisableNativeTick))
class BECORE_API UBETabButtonBase : public UBEButtonBase, public IBETabButtonInterface
{
	GENERATED_BODY()

public:

	void SetIconFromLazyObject(TSoftObjectPtr<UObject> LazyObject);
	void SetIconBrush(const FSlateBrush& Brush);

protected:

	UFUNCTION()
	virtual void SetTabLabelInfo_Implementation(const FBETabDescriptor& TabLabelInfo) override;

private:

	UPROPERTY(meta = (BindWidgetOptional))
	UCommonLazyImage* LazyImage_Icon;
};

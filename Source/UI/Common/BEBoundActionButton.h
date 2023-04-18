// Copyright Eigi Chin

#pragma once

#include "CoreMinimal.h"
#include "Input/CommonBoundActionButton.h"
#include "BEBoundActionButton.generated.h"

/**
 * 
 */
UCLASS(Abstract, meta = (DisableNativeTick))
class BECORE_API UBEBoundActionButton : public UCommonBoundActionButton
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

private:
	void HandleInputMethodChanged(ECommonInputType NewInputMethod);

	UPROPERTY(EditAnywhere, Category = "Styles")
	TSubclassOf<UCommonButtonStyle> KeyboardStyle;

	UPROPERTY(EditAnywhere, Category = "Styles")
	TSubclassOf<UCommonButtonStyle> GamepadStyle;

	UPROPERTY(EditAnywhere, Category = "Styles")
	TSubclassOf<UCommonButtonStyle> TouchStyle;
};

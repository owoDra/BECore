// Copyright owoDra

#pragma once

#include "CommonActivatableWidget.h"

#include "BEControllerDisconnectedScreen.generated.h"

class UHorizontalBox;

UCLASS(Abstract, BlueprintType, Blueprintable)
class UBEControllerDisconnectedScreen : public UCommonActivatableWidget
{
	GENERATED_BODY()

protected:
	void NativeOnActivated() override;

private:
	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* HBox_SwitchUser;
};
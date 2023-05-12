// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "GameSettingAction.h"
#include "GameSettingValueScalarDynamic.h"

#include "Containers/Array.h"
#include "UObject/UObjectGlobals.h"

#include "BESettingValueScalar_SafeZone.generated.h"

class UGameSetting;
class UObject;

//////////////////////////////////////////////////////////////////////

UCLASS()
class UBESettingValueScalarDynamic_SafeZoneValue : public UGameSettingValueScalarDynamic
{
	GENERATED_BODY()

public:
	virtual void ResetToDefault() override;
	virtual void RestoreToInitial() override;
};

UCLASS()
class UBESettingAction_SafeZoneEditor : public UGameSettingAction
{
	GENERATED_BODY()
	
public:
	UBESettingAction_SafeZoneEditor();
	virtual TArray<UGameSetting*> GetChildSettings() override;

private:
	UPROPERTY()
	UBESettingValueScalarDynamic_SafeZoneValue* SafeZoneValueSetting;
};
// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "GameSettingValue.h"

#include "EnhancedActionKeyMapping.h"
#include "GameSettingValue.h"
#include "HAL/Platform.h"
#include "InputCoreTypes.h"
#include "Internationalization/Text.h"
#include "UObject/UObjectGlobals.h"

#include "BESettingValueKeyConfig.generated.h"

class UObject;
class UPlayerMappableInputConfig;

//////////////////////////////////////////////////////////////////////

struct FKeyOption
{
	FKeyOption() = default;
	
	FEnhancedActionKeyMapping InputMapping {};
	
	const UPlayerMappableInputConfig* OwningConfig = nullptr;

	void ResetToDefault();

	/** Store the currently set FKey that this is bound to */
	void SetInitialValue(FKey InKey);

	/** Get the most recently stored initial value */
	FKey GetInitialStoredValue() const { return InitialMapping; };

private:

	/** The key that this option is bound to initially, used in case the user wants to cancel their mapping */
	FKey InitialMapping;
};

//////////////////////////////////////////////////////////////////////

UCLASS()
class UBESettingValueKeyConfig : public UGameSettingValue
{
	GENERATED_BODY()

	//======================================
	//	初期化
	//======================================
public:
	UBESettingValueKeyConfig();

protected:
	virtual void OnInitialized() override;

public:
	virtual void StoreInitial() override;

	/** Initalize this setting widget based off the given mapping */
	void SetInputData(FEnhancedActionKeyMapping& BaseMapping, const UPlayerMappableInputConfig* InOwningConfig, int32 InKeyBindSlot);

	//======================================
	//	操作
	//======================================
public:
	virtual void ResetToDefault() override;
	virtual void RestoreToInitial() override;

	bool ChangeBinding(int32 InKeyBindSlot, FKey NewKey);
	void GetAllMappedActionsFromKey(int32 InKeyBindSlot, FKey Key, TArray<FName>& OutActionNames) const;

	TArray<FKeyOption> GetMappableOptions() const { return MappableOptions; }

	FText GetSettingDisplayName() const { return MappableOptions[0].InputMapping.PlayerMappableOptions.DisplayName; }

protected:
	TArray<FKeyOption> MappableOptions;
};

// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "Engine/DeveloperSettingsBackedByCVars.h"

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "BEDeveloperPlatformSettings.generated.h"


/**
 * UBEDeveloperPlatformSettings
 * 
 *  開発するプラットフォームに対応する設定を定義する
 */
UCLASS(config = EditorPerProjectUserSettings, MinimalAPI)
class UBEDeveloperPlatformSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()
public:
	UBEDeveloperPlatformSettings();

public:
	//~UDeveloperSettings interface
	virtual FName GetCategoryName() const override;
	//~End of UDeveloperSettings interface

	FName GetPretendBaseDeviceProfile() const;
	FName GetPretendPlatformName() const;
	
#if WITH_EDITOR

public:
	//~UObject interface
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostReloadConfig(FProperty* PropertyThatWasLoaded) override;
	virtual void PostInitProperties() override;
	//~End of UObject interface

	/**
	 * OnPlayInEditorStarted
	 *
	 *  PIE を開始したときに Editor によって呼び出され、通知ポップアップを表示する
	 */
	BECORE_API void OnPlayInEditorStarted() const;

private:
	void ApplySettings();
	void ChangeActivePretendPlatform(FName NewPlatformName);

	// 最後に適用したプラットフォームエミュレーション
	FName LastAppliedPretendPlatform;

#endif 
	
private:
	UFUNCTION()
	TArray<FName> GetKnownPlatformIds() const;

	UFUNCTION()
	TArray<FName> GetKnownDeviceProfiles() const;

	void PickReasonableBaseDeviceProfile();


private:
	UPROPERTY(EditAnywhere, config, Category = "BE|Platform", meta = (Categories = "Platform.Trait"))
	FGameplayTagContainer AdditionalPlatformTraitsToEnable;

	UPROPERTY(EditAnywhere, config, Category = "BE|Platform", meta = (Categories = "Platform.Trait"))
	FGameplayTagContainer AdditionalPlatformTraitsToSuppress;

	UPROPERTY(EditAnywhere, config, Category = "BE|Platform", meta = (GetOptions = GetKnownPlatformIds))
	FName PretendPlatform;

	// The base device profile to pretend we are using when emulating device-specific device profiles applied from UBEGameDeviceSettings
	UPROPERTY(EditAnywhere, config, Category = "BE|Platform", meta = (GetOptions = GetKnownDeviceProfiles, EditCondition = bApplyDeviceProfilesInPIE))
	FName PretendBaseDeviceProfile;

	// Do we apply desktop-style frame rate settings in PIE?
	// (frame rate limits are an engine-wide setting so it's not always desirable to have enabled in the editor)
	// You may also want to disable the editor preference "Use Less CPU when in Background" if testing background frame rate limits
	UPROPERTY(EditAnywhere, config, Category = "BE|Platform", meta = (ConsoleVariable = "BE.Settings.ApplyFrameRateSettingsInPIE"))
	bool bApplyFrameRateSettingsInPIE = false;

	// Do we apply front-end specific performance options in PIE?
	// Most engine performance/scalability settings they drive are global, so if one PIE window
	// is in the front-end and the other is in-game one will win and the other gets stuck with those settings
	UPROPERTY(EditAnywhere, config, Category = "BE|Platform", meta = (ConsoleVariable = "BE.Settings.ApplyFrontEndPerformanceOptionsInPIE"))
	bool bApplyFrontEndPerformanceOptionsInPIE = false;

	// Should we apply experience/platform emulated device profiles in PIE?
	UPROPERTY(EditAnywhere, config, Category = "BE|Platform", meta = (InlineEditConditionToggle, ConsoleVariable = "BE.Settings.ApplyDeviceProfilesInPIE"))
	bool bApplyDeviceProfilesInPIE = false;
};

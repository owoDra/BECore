// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#pragma once

#include "Engine/PlatformSettings.h"
#include "Engine/DeveloperSettingsBackedByCVars.h"

#include "Performance/BEPerformanceStatTypes.h"

#include "Containers/Array.h"
#include "Containers/Set.h"
#include "Containers/UnrealString.h"
#include "GameplayTagContainer.h"
#include "HAL/Platform.h"
#include "Internationalization/Text.h"
#include "UObject/UObjectGlobals.h"

#include "BEPerformanceSettings.generated.h"

class UObject;


// Describes one platform-specific device profile variant that the user can choose from in the UI
USTRUCT()
struct FBEQualityDeviceProfileVariant
{
	GENERATED_BODY()

	// The display name for this device profile variant (visible in the options screen)
	UPROPERTY(EditAnywhere)
	FText DisplayName;

	// The suffix to append to the base device profile name for the current platform
	UPROPERTY(EditAnywhere)
	FString DeviceProfileSuffix;

	// The minimum required refresh rate to enable this mode
	// (e.g., if this is set to 120 Hz and the device is connected
	// to a 60 Hz display, it won't be available)
	UPROPERTY(EditAnywhere)
	int32 MinRefreshRate = 0;
};

// Describes a set of performance stats that the user can enable in settings,
// predicated on passing a visibility query on platform traits
USTRUCT()
struct FBEPerformanceStatGroup
{
	GENERATED_BODY()

	// A query on platform traits to determine whether or not it will be possible
	// to show a set of stats
	UPROPERTY(EditAnywhere, meta=(Categories = "Input,Platform.Trait"))
	FGameplayTagQuery VisibilityQuery;

	// The set of stats to allow if the query passes
	UPROPERTY(EditAnywhere)
	TSet<EBEDisplayablePerformanceStat> AllowedStats;
};

// How hare frame pacing and overall graphics settings controlled/exposed for the platform?
UENUM()
enum class EBEFramePacingMode : uint8
{
	// Manual frame rate limits, user is allowed to choose whether or not to lock to vsync
	DesktopStyle,

	// Limits handled by choosing present intervals driven by device profiles
	ConsoleStyle
};

UCLASS(config=Game, defaultconfig)
class UBEPlatformSpecificRenderingSettings : public UPlatformSettings
{
	GENERATED_BODY()

public:
	UBEPlatformSpecificRenderingSettings();

	// Helper method to get the performance settings object, directed via platform settings
	static const UBEPlatformSpecificRenderingSettings* Get();

public:
	// The default variant suffix to append, should typically be a member of
	// UserFacingDeviceProfileOptions unless there is only one for the current platform
	//
	// Note that this will usually be set from platform-specific ini files, not via the UI
	UPROPERTY(EditAnywhere, Config, Category=DeviceProfiles)
	FString DefaultDeviceProfileSuffix;

	// The list of device profile variations to allow users to choose from in settings
	//
	// These should be sorted from slowest to fastest by target frame rate:
	//   If the current display doesn't support a user chosen refresh rate, we'll try
	//   previous entries until we find one that works
	//
	// Note that this will usually be set from platform-specific ini files, not via the UI
	UPROPERTY(EditAnywhere, Config, Category=DeviceProfiles)
	TArray<FBEQualityDeviceProfileVariant> UserFacingDeviceProfileOptions;

	// Does the platform support granular video quality settings?
	UPROPERTY(EditAnywhere, Config, Category=VideoSettings)
	bool bSupportsGranularVideoQualitySettings = true;

	// Does the platform support running the automatic quality benchmark (typically this should only be true if bSupportsGranularVideoQualitySettings is also true)
	UPROPERTY(EditAnywhere, Config, Category=VideoSettings)
	bool bSupportsAutomaticVideoQualityBenchmark = true;

	// How is frame pacing controlled
	UPROPERTY(EditAnywhere, Config, Category=VideoSettings)
	EBEFramePacingMode FramePacingMode = EBEFramePacingMode::DesktopStyle;

	// Potential frame rates to display for mobile
	// Note: This is further limited by BE.DeviceProfile.Mobile.MaxFrameRate from the
	// platform-specific device profile and what the platform frame pacer reports as supported
	UPROPERTY(EditAnywhere, Config, Category=VideoSettings, meta=(EditCondition="FramePacingMode==EBEFramePacingMode::MobileStyle", ForceUnits=Hz))
	TArray<int32> MobileFrameRateLimits;
};

//////////////////////////////////////////////////////////////////////

/**
 * Project-specific performance profile settings.
 */
UCLASS(config=Game, defaultconfig, meta=(DisplayName="BE Performance Settings"))
class UBEPerformanceSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:
	UBEPerformanceSettings();

private:
	// This is a special helper to expose the per-platform settings so they can be edited in the project settings
	// It never needs to be directly accessed
	UPROPERTY(EditAnywhere, Category = "PlatformSpecific")
	FPerPlatformSettings PerPlatformSettings;

public:
	// The list of frame rates to allow users to choose from in the various
	// "frame rate limit" video settings on desktop platforms
	UPROPERTY(EditAnywhere, Config, Category=Performance, meta=(ForceUnits=Hz))
	TArray<int32> DesktopFrameRateLimits;

	// The list of performance stats that can be enabled in Options by the user
	UPROPERTY(EditAnywhere, Config, Category=Stats)
	TArray<FBEPerformanceStatGroup> UserFacingPerformanceStats;
};
// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#include "BEPerformanceSettings.h"

#include "Engine/PlatformSettings.h"
#include "Engine/PlatformSettingsManager.h"
#include "Misc/AssertionMacros.h"
#include "Misc/EnumRange.h"
#include "UObject/NameTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEPerformanceSettings)


//////////////////////////////////////////////////////////////////////

UBEPlatformSpecificRenderingSettings::UBEPlatformSpecificRenderingSettings()
{
	MobileFrameRateLimits.Append({ 20, 30, 45, 60, 90, 120 });
}

const UBEPlatformSpecificRenderingSettings* UBEPlatformSpecificRenderingSettings::Get()
{
	UBEPlatformSpecificRenderingSettings* Result = UPlatformSettingsManager::Get().GetSettingsForPlatform<ThisClass>();
	check(Result);
	return Result;
}

//////////////////////////////////////////////////////////////////////

UBEPerformanceSettings::UBEPerformanceSettings()
{
	PerPlatformSettings.Initialize(UBEPlatformSpecificRenderingSettings::StaticClass());

	CategoryName = TEXT("Game");

	DesktopFrameRateLimits.Append({ 30, 60, 120, 144, 160, 165, 180, 200, 240, 360 });

	// Default to all stats are allowed
	FBEPerformanceStatGroup& StatGroup = UserFacingPerformanceStats.AddDefaulted_GetRef();
	for (EBEDisplayablePerformanceStat PerfStat : TEnumRange<EBEDisplayablePerformanceStat>())
	{
		StatGroup.AllowedStats.Add(PerfStat);
	}
}

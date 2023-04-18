// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BESettingValueDiscrete_PerfStat.h"

#include "Settings/BESettingsLocal.h"
#include "Performance/BEPerformanceSettings.h"

#include "CommonUIVisibilitySubsystem.h"
#include "Containers/Set.h"
#include "Containers/UnrealString.h"
#include "GameSettingFilterState.h"
#include "GameplayTagContainer.h"
#include "Internationalization/Internationalization.h"
#include "Templates/SharedPointer.h"
#include "Templates/UnrealTemplate.h"
#include "UObject/NameTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BESettingValueDiscrete_PerfStat)

class ULocalPlayer;

//////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "BESettings"

class FGameSettingEditCondition_PerfStatAllowed : public FGameSettingEditCondition
{
public:
	static TSharedRef<FGameSettingEditCondition_PerfStatAllowed> Get(EBEDisplayablePerformanceStat Stat)
	{
		return MakeShared<FGameSettingEditCondition_PerfStatAllowed>(Stat);
	}

	FGameSettingEditCondition_PerfStatAllowed(EBEDisplayablePerformanceStat Stat)
		: AssociatedStat(Stat)
	{
	}

	//~FGameSettingEditCondition interface
	virtual void GatherEditState(const ULocalPlayer* InLocalPlayer, FGameSettingEditableState& InOutEditState) const override
	{
		const FGameplayTagContainer& VisibilityTags = UCommonUIVisibilitySubsystem::GetChecked(InLocalPlayer)->GetVisibilityTags();

		bool bCanShowStat = false;
		for (const FBEPerformanceStatGroup& Group : GetDefault<UBEPerformanceSettings>()->UserFacingPerformanceStats) //@TODO: Move this stuff to per-platform instead of doing vis queries too?
		{
			if (Group.AllowedStats.Contains(AssociatedStat))
			{
				const bool bShowGroup = (Group.VisibilityQuery.IsEmpty() || Group.VisibilityQuery.Matches(VisibilityTags));
				if (bShowGroup)
				{
					bCanShowStat = true;
					break;
				}
			}
		}

		if (!bCanShowStat)
		{
			InOutEditState.Hide(TEXT("Stat is not listed in UBEPerformanceSettings or is suppressed by current platform traits"));
		}
	}
	//~End of FGameSettingEditCondition interface

private:
	EBEDisplayablePerformanceStat AssociatedStat;
};

//////////////////////////////////////////////////////////////////////

UBESettingValueDiscrete_PerfStat::UBESettingValueDiscrete_PerfStat()
{
}

void UBESettingValueDiscrete_PerfStat::SetStat(EBEDisplayablePerformanceStat InStat)
{
	StatToDisplay = InStat;
	SetDevName(FName(*FString::Printf(TEXT("PerfStat_%d"), (int32)StatToDisplay)));
	AddEditCondition(FGameSettingEditCondition_PerfStatAllowed::Get(StatToDisplay));
}

void UBESettingValueDiscrete_PerfStat::AddMode(FText&& Label, EBEStatDisplayMode Mode)
{
	Options.Emplace(MoveTemp(Label));
	DisplayModes.Add(Mode);
}

void UBESettingValueDiscrete_PerfStat::OnInitialized()
{
	Super::OnInitialized();

	AddMode(LOCTEXT("PerfStatDisplayMode_None", "None"), EBEStatDisplayMode::Hidden);
	AddMode(LOCTEXT("PerfStatDisplayMode_TextOnly", "Text Only"), EBEStatDisplayMode::TextOnly);
	AddMode(LOCTEXT("PerfStatDisplayMode_GraphOnly", "Graph Only"), EBEStatDisplayMode::GraphOnly);
	AddMode(LOCTEXT("PerfStatDisplayMode_TextAndGraph", "Text and Graph"), EBEStatDisplayMode::TextAndGraph);
}

void UBESettingValueDiscrete_PerfStat::StoreInitial()
{
	const UBESettingsLocal* Settings = UBESettingsLocal::Get();
	InitialMode = Settings->GetPerfStatDisplayState(StatToDisplay);
}

void UBESettingValueDiscrete_PerfStat::ResetToDefault()
{
	UBESettingsLocal* Settings = UBESettingsLocal::Get();
	Settings->SetPerfStatDisplayState(StatToDisplay, EBEStatDisplayMode::Hidden);
	NotifySettingChanged(EGameSettingChangeReason::ResetToDefault);
}

void UBESettingValueDiscrete_PerfStat::RestoreToInitial()
{
	UBESettingsLocal* Settings = UBESettingsLocal::Get();
	Settings->SetPerfStatDisplayState(StatToDisplay, InitialMode);
	NotifySettingChanged(EGameSettingChangeReason::RestoreToInitial);
}

void UBESettingValueDiscrete_PerfStat::SetDiscreteOptionByIndex(int32 Index)
{
	if (DisplayModes.IsValidIndex(Index))
	{
		const EBEStatDisplayMode DisplayMode = DisplayModes[Index];

		UBESettingsLocal* Settings = UBESettingsLocal::Get();
		Settings->SetPerfStatDisplayState(StatToDisplay, DisplayMode);
	}
	NotifySettingChanged(EGameSettingChangeReason::Change);
}

int32 UBESettingValueDiscrete_PerfStat::GetDiscreteOptionIndex() const
{
	const UBESettingsLocal* Settings = UBESettingsLocal::Get();
	return DisplayModes.Find(Settings->GetPerfStatDisplayState(StatToDisplay));
}

TArray<FText> UBESettingValueDiscrete_PerfStat::GetDiscreteOptions() const
{
	return Options;
}

#undef LOCTEXT_NAMESPACE
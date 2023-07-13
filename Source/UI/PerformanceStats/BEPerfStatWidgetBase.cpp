// Copyright owoDra

#include "BEPerfStatWidgetBase.h"
#include "Performance/BEPerformanceStatSubsystem.h"

//////////////////////////////////////////////////////////////////////
// UBEPerfStatWidgetBase

UBEPerfStatWidgetBase::UBEPerfStatWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

double UBEPerfStatWidgetBase::FetchStatValue()
{
	if (CachedStatSubsystem == nullptr)
	{
		if (UWorld* World = GetWorld())
		{
			if (UGameInstance* GameInstance = World->GetGameInstance())
			{
				CachedStatSubsystem = GameInstance->GetSubsystem<UBEPerformanceStatSubsystem>();
			}
		}
	}

	if (CachedStatSubsystem)
	{
		return CachedStatSubsystem->GetCachedStat(StatToDisplay);
	}
	else
	{
		return 0.0;
	}
}

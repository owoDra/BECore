// Copyright 2022 RutenStudio

#include "BETaskWaitOneFrame.h"


UBETaskWaitOneFrame::UBETaskWaitOneFrame(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UBETaskWaitOneFrame* UBETaskWaitOneFrame::CreateWaitOneFrame(UGameplayAbility* OwningAbility)
{
	UBETaskWaitOneFrame* MyObj = NewAbilityTask<UBETaskWaitOneFrame>(OwningAbility);
	return MyObj;
}

void UBETaskWaitOneFrame::Activate()
{
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UBETaskWaitOneFrame::OnDelayFinish);
}

void UBETaskWaitOneFrame::OnDelayFinish()
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnFinish.Broadcast();
	}
	EndTask();
}

// Copyright 2022 RutenStudio

#include "BETaskRepeatWithDuration.h"


UBETaskRepeatWithDuration::UBETaskRepeatWithDuration(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UBETaskRepeatWithDuration* UBETaskRepeatWithDuration::CreateRepeatWithDuration(UGameplayAbility* OwningAbility, float Duration)
{
	UBETaskRepeatWithDuration* MyObj = NewAbilityTask<UBETaskRepeatWithDuration>(OwningAbility);

	MyObj->DesiredDuration = Duration;

	return MyObj;
}

void UBETaskRepeatWithDuration::Activate()
{
	StartTime = GetWorld()->GetTimeSeconds();
	LastActionTime = StartTime;

	if (DesiredDuration > 0)
	{
		PerformAction();
	}
	else
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnFinished.Broadcast(0);
		}
		EndTask();
	}
}

void UBETaskRepeatWithDuration::PerformAction()
{
	CurrentTime = GetWorld()->GetTimeSeconds();
	LastTimeDelta = CurrentTime - LastActionTime;
	LastActionTime = CurrentTime;

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnPerformAction.Broadcast(LastTimeDelta);
	}
	else
	{
		EndTask();
		return;
	}

	if (CurrentTime - StartTime >= DesiredDuration)
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnFinished.Broadcast(LastTimeDelta);
		}
		EndTask();
	}
	else
	{
		TimerRepeatAction = GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UBETaskRepeatWithDuration::PerformAction);
	}
}

FString UBETaskRepeatWithDuration::GetDebugString() const
{
	return FString::Printf(TEXT("RepeatAction. DeltaSeconds: %.2f"), LastTimeDelta);
}

void UBETaskRepeatWithDuration::OnDestroy(bool AbilityIsEnding)
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(TimerRepeatAction);
	}

	Super::OnDestroy(AbilityIsEnding);
}

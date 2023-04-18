// Copyright 2022 RutenStudio

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "BETaskRepeatWithDuration.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRepeatedActionDelegate, float, Delta);


// ========================================
//   Repeat With Duration
// ========================================
UCLASS()
class BECORE_API UBETaskRepeatWithDuration : public UAbilityTask
{
	GENERATED_UCLASS_BODY()

	// ====================================
	//   Variables
	// ====================================

	UPROPERTY(BlueprintAssignable)
		FRepeatedActionDelegate	OnPerformAction;

	UPROPERTY(BlueprintAssignable)
		FRepeatedActionDelegate	OnFinished;

protected:
	float StartTime;
	float LastActionTime;
	float LastTimeDelta;
	float CurrentTime;

	float DesiredDuration;

	FTimerHandle TimerRepeatAction;
public:


	// ====================================
	//   Functions
	// ====================================

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE", DisplayName = "Repeat With Duration"))
		static UBETaskRepeatWithDuration* CreateRepeatWithDuration(UGameplayAbility* OwningAbility, float Duration);

	virtual void Activate() override;

	void PerformAction();

	virtual FString GetDebugString() const override;

protected:

	virtual void OnDestroy(bool AbilityIsEnding) override;
	
};

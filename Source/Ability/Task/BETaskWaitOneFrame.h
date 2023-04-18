// Copyright 2022 RutenStudio

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "BETaskWaitOneFrame.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWaitOneFrameDelegate);


// ========================================
//   Wait One Frame
// ========================================
UCLASS()
class BECORE_API UBETaskWaitOneFrame : public UAbilityTask
{
	GENERATED_UCLASS_BODY()

	// ====================================
	//   Variables
	// ====================================

	UPROPERTY(BlueprintAssignable)
		FWaitOneFrameDelegate OnFinish;


	// ====================================
	//   Functions
	// ====================================

	// Wait one frame.
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE", DisplayName = "Wait One Frame"))
		static UBETaskWaitOneFrame* CreateWaitOneFrame(UGameplayAbility* OwningAbility);

	virtual void Activate() override;

private:
	void OnDelayFinish();
};

// Copyright Eigi Chin

#pragma once

#include "CoreMinimal.h"
#include "GameUIManagerSubsystem.h"

#include "BEUIManagerSubsystem.generated.h"

UCLASS()
class UBEUIManagerSubsystem : public UGameUIManagerSubsystem
{
	GENERATED_BODY()

public:

	UBEUIManagerSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	bool Tick(float DeltaTime);
	void SyncRootLayoutVisibilityToShowHUD();
	
	FTSTicker::FDelegateHandle TickHandle;
};
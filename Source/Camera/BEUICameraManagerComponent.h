// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "Components/ActorComponent.h"

#include "Camera/PlayerCameraManager.h"
#include "Misc/AssertionMacros.h"
#include "UObject/UObjectGlobals.h"

#include "BEUICameraManagerComponent.generated.h"

class AActor;
class AHUD;
class APlayerController;
class FDebugDisplayInfo;
class UCanvas;
class UObject;


UCLASS( Transient, Within=BEPlayerCameraManager )
class UBEUICameraManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	static UBEUICameraManagerComponent* GetComponent(APlayerController* PC);

public:
	UBEUICameraManagerComponent();	
	virtual void InitializeComponent() override;

	bool IsSettingViewTarget() const { return bUpdatingViewTarget; }
	AActor* GetViewTarget() const { return ViewTarget; }
	void SetViewTarget(AActor* InViewTarget, FViewTargetTransitionParams TransitionParams = FViewTargetTransitionParams());

	bool NeedsToUpdateViewTarget() const;
	void UpdateViewTarget(struct FTViewTarget& OutVT, float DeltaTime);

	void OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos);

private:
	UPROPERTY(Transient)
	TObjectPtr<AActor> ViewTarget;
	
	UPROPERTY(Transient)
	bool bUpdatingViewTarget;
};

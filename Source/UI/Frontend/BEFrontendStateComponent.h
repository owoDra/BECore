// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "Components/GameStateComponent.h"

#include "ControlFlowNode.h"
#include "Engine/EngineTypes.h"
#include "HAL/Platform.h"
#include "LoadingProcessInterface.h"
#include "Templates/SharedPointer.h"
#include "UObject/SoftObjectPtr.h"
#include "UObject/UObjectGlobals.h"

#include "BEFrontendStateComponent.generated.h"

class FControlFlow;
class FString;
class FText;
class UObject;
struct FFrame;

enum class ECommonUserOnlineContext : uint8;
enum class ECommonUserPrivilege : uint8;
class UCommonActivatableWidget;
class UBEExperienceDefinition;
class UCommonUserInfo;

////////////////////////////////////////////////

UCLASS(Abstract)
class UBEFrontendStateComponent : public UGameStateComponent, public ILoadingProcessInterface
{
	GENERATED_BODY()

	//=========================================
	//  初期化
	//=========================================
public:
	UBEFrontendStateComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UActorComponent interface
	virtual void BeginPlay() override;
	//~End of UActorComponent interface

private:
	void OnExperienceLoaded(const UBEExperienceDefinition* Experience);

	//=========================================
	//  フロー
	//=========================================
private:
	void FlowStep_CheckUserInitialization(FControlFlowNodeRef SubFlow);
	void FlowStep_TryStartUp(FControlFlowNodeRef SubFlow);
	void FlowStep_TryJoinRequestedSession(FControlFlowNodeRef SubFlow);
	void FlowStep_TryShowMainScreen(FControlFlowNodeRef SubFlow);

private:
	UPROPERTY(EditAnywhere, Category = UI)
		TSoftClassPtr<UCommonActivatableWidget> StartupScreenClass;

	UPROPERTY(EditAnywhere, Category = UI)
		TSoftClassPtr<UCommonActivatableWidget> MainScreenClass;

	UPROPERTY(EditAnywhere, Category = Gameplay)
		bool bPlayerMustInitializeForOnline = false;

private:
	TSharedPtr<FControlFlow> FrontEndFlow;

	// If set, this is the in-progress press start screen task
	FControlFlowNodePtr InProgressPressStartScreen;

	FDelegateHandle OnJoinSessionCompleteEventHandle;


	//=========================================
	//  ユーティリティ
	//=========================================
public:
	//~ILoadingProcessInterface interface
	virtual bool ShouldShowLoadingScreen(FString& OutReason) const override;
	//~End of ILoadingProcessInterface

private:
	bool bShouldShowLoadingScreen = true;
};

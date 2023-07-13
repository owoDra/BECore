// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#pragma once

#include "GameFramework/Info.h"

#include "Engine/EngineTypes.h"
#include "HAL/Platform.h"
#include "System/GameplayTagStack.h"
#include "UObject/UObjectGlobals.h"

#include "BETeamInfoBase.generated.h"

class UBETeamCreationComponent;
class UBETeamSubsystem;
class UObject;
struct FFrame;


UCLASS(Abstract)
class ABETeamInfoBase : public AInfo
{
	GENERATED_BODY()

public:
	ABETeamInfoBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	int32 GetTeamId() const { return TeamId; }

	//~AActor interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of AActor interface

protected:
	virtual void RegisterWithTeamSubsystem(UBETeamSubsystem* Subsystem);
	void TryRegisterWithTeamSubsystem();

private:
	void SetTeamId(int32 NewTeamId);

	UFUNCTION()
	void OnRep_TeamId();

public:
	friend UBETeamCreationComponent;

	UPROPERTY(Replicated)
	FGameplayTagStackContainer TeamTags;

private:
	UPROPERTY(ReplicatedUsing=OnRep_TeamId)
	int32 TeamId;
};

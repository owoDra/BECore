// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#pragma once

#include "CommonLocalPlayer.h"

#include "Team/BETeamAgentInterface.h"

#include "AudioMixerBlueprintLibrary.h"
#include "Containers/UnrealString.h"
#include "GenericTeamAgentInterface.h"
#include "HAL/Platform.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/WeakObjectPtrTemplates.h"

#include "BELocalPlayer.generated.h"

class APlayerController;
class UInputMappingContext;
class UBEGameDeviceSettings;
class UBEGameSharedSettings;
class UObject;
class UWorld;
struct FFrame;
struct FSwapAudioOutputResult;


/**
 * UBELocalPlayer
 * 
 *  Player そのものが持つ情報(ゲーム設定、デバイス設定、入力設定、など)を保有する
 */
UCLASS()
class BECORE_API UBELocalPlayer : public UCommonLocalPlayer, public IBETeamAgentInterface
{
	GENERATED_BODY()
public:
	UBELocalPlayer() {}

public:
	//~UObject interface
	virtual void PostInitProperties() override;
	//~End of UObject interface

	//~UPlayer interface
	virtual void SwitchController(class APlayerController* PC) override;
	//~End of UPlayer interface

	//~ULocalPlayer interface
	virtual bool SpawnPlayActor(const FString& URL, FString& OutError, UWorld* InWorld) override;
	virtual void InitOnlineSession() override;
	//~End of ULocalPlayer interface

	UFUNCTION()
	UBEGameDeviceSettings* GetDeviceSettings() const;

	UFUNCTION()
	UBEGameSharedSettings* GetSharedSettings() const;

private:
	UPROPERTY(Transient)
	mutable TObjectPtr<UBEGameSharedSettings> GameSharedSettings;

	UPROPERTY(Transient)
	mutable TObjectPtr<const UInputMappingContext> InputMappingContext;

	UPROPERTY()
	TWeakObjectPtr<APlayerController> LastBoundPC;

protected:
	void OnAudioOutputDeviceChanged(const FString& InAudioOutputDeviceId);
	
	UFUNCTION()
	void OnCompletedAudioDeviceSwap(const FSwapAudioOutputResult& SwapResult);


public:
	//~IBETeamAgentInterface interface
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual FOnBETeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	//~End of IBETeamAgentInterface interface

private:
	void OnPlayerControllerChanged(APlayerController* NewController);

	UFUNCTION()
	void OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

private:
	UPROPERTY()
	FOnBETeamIndexChangedDelegate OnTeamChangedDelegate;
};

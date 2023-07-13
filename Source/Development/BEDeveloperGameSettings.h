// Copyright owoDra

#pragma once

#include "Engine/DeveloperSettingsBackedByCVars.h"

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "BEDeveloperGameSettings.generated.h"

class UBEExperienceDefinition;


/**
 * UBEDeveloperGameSettings
 *
 * ゲームのテストプレイを行う際の開発者用のゲーム設定を定義する
 */
UCLASS(config = EditorPerProjectUserSettings, MinimalAPI)
class UBEDeveloperGameSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()
public:
	UBEDeveloperGameSettings();

public:
	//~UDeveloperSettings interface
	virtual FName GetCategoryName() const override;
	//~End of UDeveloperSettings interface

#if WITH_EDITOR

	//~UObject interface
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostReloadConfig(FProperty* PropertyThatWasLoaded) override;
	virtual void PostInitProperties() override;
	//~End of UObject interface

	/**
	 * OnPlayInEditorStarted
	 *
	 *  PIE を開始したときに Editor によって呼び出され、通知ポップアップを表示する
	 */
	BECORE_API void OnPlayInEditorStarted() const;

#endif 


public:
	// PIE でゲームを実行した際に WorldSettings で設定した Experience を無視して適応する Experience
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, config, Category = "BE|Game", meta = (AllowedTypes = "BEExperienceDefinition"))
	FPrimaryAssetId ExperienceOverride;

	// PIE でゲームを実行したときに全ての過程をテストするか
	// false の場合、一部のロードが省かれる。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, config, Category = "BE|Game")
	bool bTestFullGameFlowInPIE = false;

	// PlayerBot が攻撃を行うかどうか
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, config, Category = "BE|Game|Bot")
	bool bAllowPlayerBotsToAttack = true;

	// 常にゲームコントローラーの振動などを行うかどうか
	// 通常、コントローラー以外のデバイスを使用している場合、コントローラーが接続されていても振動を行わない
	UPROPERTY(config, EditAnywhere, Category = "BE|Game", meta = (ConsoleVariable = "BEPC.ShouldAlwaysPlayForceFeedback"))
	bool bShouldAlwaysPlayForceFeedback = false;

	// Log に GameplayMessageSubsystem を用いた通知を表示するか
	UPROPERTY(config, EditAnywhere, Category = "BE|Game", meta = (ConsoleVariable = "GameplayMessageSubsystem.LogMessages"))
	bool LogGameplayMessages = false;


#if WITH_EDITORONLY_DATA
	// 素早く Map を開けるように登録できるリスト
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = Maps, meta = (AllowedClasses = "/Script/Engine.World"))
		TArray<FSoftObjectPath> QuickAccessEditorMaps;
#endif


public:
	// ゲームの共有設定のクラス
	UPROPERTY(config, EditDefaultsOnly, BlueprintReadOnly, Category = "BE|GameSettings", meta = (AllowedClasses = "/Script/BECore.BEGameSharedSettings"))
	FSoftClassPath SharedSettingClass;

	// ゲームの共有設定のクラス
	UPROPERTY(config, EditDefaultsOnly, BlueprintReadOnly, Category = "BE|GameSettings", meta = (AllowedClasses = "/Script/BECore.BEGameSettingRegistry"))
	FSoftClassPath SettingRegistryClass;
};

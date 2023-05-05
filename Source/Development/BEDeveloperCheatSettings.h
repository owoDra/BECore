// Copyright Eigi Chin

#pragma once

#include "Engine/DeveloperSettingsBackedByCVars.h"

#include "CoreMinimal.h"

#include "BEDeveloperCheatSettings.generated.h"


/**
 * ECheatExecutionTime
 * 
 * 開発者用チートの自動実行をどのタイミングで行うか
 */
UENUM()
enum class ECheatExecutionTime
{
	// CheatManager が作成されたときに実行
	OnCheatManagerCreated,

	// Player が Pawn を所有したときに実行
	OnPlayerPawnPossession
};


/**
 * FBECheatToRun
 *
 * 開発者用チートの自動実行のためのエントリー情報
 */
USTRUCT()
struct FBECheatToRun
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	ECheatExecutionTime Phase = ECheatExecutionTime::OnPlayerPawnPossession;

	UPROPERTY(EditAnywhere)
	FString Cheat;
};


/**
 * UBEDeveloperCheatSettings
 *
 * 開発者用チート関係の設定を定義する
 */
UCLASS(config = EditorPerProjectUserSettings, MinimalAPI)
class UBEDeveloperCheatSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()
public:
	UBEDeveloperCheatSettings();

public:
	//~UDeveloperSettings interface
	virtual FName GetCategoryName() const override;
	//~End of UDeveloperSettings interface
	
#if WITH_EDITOR //~UObject interface
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostReloadConfig(FProperty* PropertyThatWasLoaded) override;
	virtual void PostInitProperties() override;
#endif //~End of UObject interface
	

public:
	// 開発者用チートの自動実行リスト
	UPROPERTY(config, EditAnywhere, Category = "BE|Cheat")
	TArray<FBECheatToRun> CheatsToRun;
};

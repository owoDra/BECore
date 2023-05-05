// Copyright Eigi Chin

#pragma once

#include "Engine/DeveloperSettingsBackedByCVars.h"

#include "CoreMinimal.h"

#include "BEDeveloperCheatSettings.generated.h"


/**
 * ECheatExecutionTime
 * 
 * �J���җp�`�[�g�̎������s���ǂ̃^�C�~���O�ōs����
 */
UENUM()
enum class ECheatExecutionTime
{
	// CheatManager ���쐬���ꂽ�Ƃ��Ɏ��s
	OnCheatManagerCreated,

	// Player �� Pawn �����L�����Ƃ��Ɏ��s
	OnPlayerPawnPossession
};


/**
 * FBECheatToRun
 *
 * �J���җp�`�[�g�̎������s�̂��߂̃G���g���[���
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
 * �J���җp�`�[�g�֌W�̐ݒ���`����
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
	// �J���җp�`�[�g�̎������s���X�g
	UPROPERTY(config, EditAnywhere, Category = "BE|Cheat")
	TArray<FBECheatToRun> CheatsToRun;
};

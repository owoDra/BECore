// Copyright Eigi Chin

#pragma once

#include "Engine/DeveloperSettingsBackedByCVars.h"

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "BEDeveloperGameSettings.generated.h"

class UBEExperienceDefinition;


/**
 * UBEDeveloperGameSettings
 *
 * �Q�[���̃e�X�g�v���C���s���ۂ̊J���җp�̃Q�[���ݒ���`����
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
	 *  PIE ���J�n�����Ƃ��� Editor �ɂ���ČĂяo����A�ʒm�|�b�v�A�b�v��\������
	 */
	BECORE_API void OnPlayInEditorStarted() const;

#endif 


public:
	// PIE �ŃQ�[�������s�����ۂ� WorldSettings �Őݒ肵�� Experience �𖳎����ēK������ Experience
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, config, Category = "BE|Game", meta = (AllowedTypes = "BEExperienceDefinition"))
	FPrimaryAssetId ExperienceOverride;

	// PIE �ŃQ�[�������s�����Ƃ��ɑS�Ẳߒ����e�X�g���邩
	// false �̏ꍇ�A�ꕔ�̃��[�h���Ȃ����B
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, config, Category = "BE|Game")
	bool bTestFullGameFlowInPIE = false;

	// PlayerBot ���U�����s�����ǂ���
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, config, Category = "BE|Game|Bot")
	bool bAllowPlayerBotsToAttack = true;

	// ��ɃQ�[���R���g���[���[�̐U���Ȃǂ��s�����ǂ���
	// �ʏ�A�R���g���[���[�ȊO�̃f�o�C�X���g�p���Ă���ꍇ�A�R���g���[���[���ڑ�����Ă��Ă��U�����s��Ȃ�
	UPROPERTY(config, EditAnywhere, Category = "BE|Game", meta = (ConsoleVariable = "BEPC.ShouldAlwaysPlayForceFeedback"))
	bool bShouldAlwaysPlayForceFeedback = false;

	// Log �� GameplayMessageSubsystem ��p�����ʒm��\�����邩
	UPROPERTY(config, EditAnywhere, Category = "BE|Game", meta = (ConsoleVariable = "GameplayMessageSubsystem.LogMessages"))
	bool LogGameplayMessages = false;


#if WITH_EDITORONLY_DATA
	// �f���� Map ���J����悤�ɓo�^�ł��郊�X�g
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = Maps, meta = (AllowedClasses = "/Script/Engine.World"))
		TArray<FSoftObjectPath> QuickAccessEditorMaps;
#endif


public:
	// �Q�[���̋��L�ݒ�̃N���X
	UPROPERTY(config, EditDefaultsOnly, BlueprintReadOnly, Category = "BE|GameSettings", meta = (AllowedClasses = "/Script/BECore.BEGameSharedSettings"))
	FSoftClassPath SharedSettingClass;

	// �Q�[���̋��L�ݒ�̃N���X
	UPROPERTY(config, EditDefaultsOnly, BlueprintReadOnly, Category = "BE|GameSettings", meta = (AllowedClasses = "/Script/BECore.BEGameSettingRegistry"))
	FSoftClassPath SettingRegistryClass;
};

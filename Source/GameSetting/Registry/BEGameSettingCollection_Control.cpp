// Copyright owoDra

#include "BEGameSettingRegistry.h"

#include "Player/BELocalPlayer.h"

#include "GameSettingCollection.h"


#define LOCTEXT_NAMESPACE "BE"

////////////////////////////////////////////////////////////
//	コントロール設定

UGameSettingCollection* UBEGameSettingRegistry::InitializeControlSettings(UBELocalPlayer* InLocalPlayer)
{
	UGameSettingCollection* Screen = NewObject<UGameSettingCollection>();
	Screen->SetDevName(TEXT("ControlCollection"));
	Screen->SetDisplayName(LOCTEXT("ControlCollection_Name", "Control"));
	Screen->Initialize(InLocalPlayer);

	///////////////////////////////////////////////////////
	//	キーバインド設定

	AddKeyBindSettingsSection(Screen, InLocalPlayer);


	///////////////////////////////////////////////////////
	//	共通設定

	AddControllerSharedSettingsSection(Screen, InLocalPlayer);


	///////////////////////////////////////////////////////
	//	キーボードマウス設定
	
	AddKeyboardMouseSettingsSection(Screen, InLocalPlayer);


	///////////////////////////////////////////////////////
	//	コントローラー設定

	AddGamepadSettingsSection(Screen, InLocalPlayer);

	return Screen;
}

#undef LOCTEXT_NAMESPACE

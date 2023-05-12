// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BEGameSettingRegistry.h"

#include "Player/BELocalPlayer.h"

#include "GameSettingCollection.h"


#define LOCTEXT_NAMESPACE "BE"

////////////////////////////////////////
//	ゲームプレイ設定

UGameSettingCollection* UBEGameSettingRegistry::InitializeGameplaySettings(UBELocalPlayer* InLocalPlayer)
{
	UGameSettingCollection* Screen = NewObject<UGameSettingCollection>();
	Screen->SetDevName(TEXT("GameplayCollection"));
	Screen->SetDisplayName(LOCTEXT("GameplayCollection_Name", "Gameplay"));
	Screen->Initialize(InLocalPlayer);

	////////////////////////////////////////
	//	アクセシビリティ設定
	
	AddAccessibilitySettingsSection(Screen, InLocalPlayer);


	////////////////////////////////////////
	//	情報表示設定
	
	AddPrefStatSettingsSection(Screen, InLocalPlayer);

	return Screen;
}

#undef LOCTEXT_NAMESPACE

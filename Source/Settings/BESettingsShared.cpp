// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BESettingsShared.h"
#include "Framework/Application/SlateApplication.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "Player/BELocalPlayer.h"
#include "Internationalization/Culture.h"
#include "Internationalization/Internationalization.h"

static FString SHARED_SETTINGS_SLOT_NAME = TEXT("SharedGameSettings");

////////////////////////////////////////
//	コンソール変数定義

namespace BESettingsSharedCVars
{
	static float DefaultGamepadLeftStickInnerDeadZone = 0.25f;
	static FAutoConsoleVariableRef CVarGamepadLeftStickInnerDeadZone(
		TEXT("gpad.DefaultLeftStickInnerDeadZone"),
		DefaultGamepadLeftStickInnerDeadZone,
		TEXT("Gamepad left stick inner deadzone")
	);

	static float DefaultGamepadRightStickInnerDeadZone = 0.25f;
	static FAutoConsoleVariableRef CVarGamepadRightStickInnerDeadZone(
		TEXT("gpad.DefaultRightStickInnerDeadZone"),
		DefaultGamepadRightStickInnerDeadZone,
		TEXT("Gamepad right stick inner deadzone")
	);	
}


////////////////////////////////////////
//	UBESettingsShared : 初期化

#define LOCTEXT_NAMESPACE "BE"

UBESettingsShared::UBESettingsShared()
{
	FInternationalization::Get().OnCultureChanged().AddUObject(this, &ThisClass::OnCultureChanged);

	GamepadMoveStickDeadZone = BESettingsSharedCVars::DefaultGamepadLeftStickInnerDeadZone;
	GamepadLookStickDeadZone = BESettingsSharedCVars::DefaultGamepadRightStickInnerDeadZone;

	ReticleProfiles.Empty();
	ReticleProfiles.EmplaceAt(0, FReticle());
	ReticleProfiles.EmplaceAt(1, FReticle());
	ReticleProfiles.EmplaceAt(2, FReticle());
	ReticleProfiles.EmplaceAt(3, FReticle());
	ReticleProfiles.EmplaceAt(4, FReticle());
}

void UBESettingsShared::Initialize(ULocalPlayer* LocalPlayer)
{
	check(LocalPlayer);
	
	OwningPlayer = Cast<UBELocalPlayer>(LocalPlayer);
}


////////////////////////////////////////
//	UBESettingsShared : データ

// ===== 色覚特性設定 =====

void UBESettingsShared::SetColorBlindStrength(int32 InColorBlindStrength)
{
	InColorBlindStrength = FMath::Clamp(InColorBlindStrength, 0, 10);
	if (ColorBlindStrength != InColorBlindStrength)
	{
		ColorBlindStrength = InColorBlindStrength;
		FSlateApplication::Get().GetRenderer()->SetColorVisionDeficiencyType(
			(EColorVisionDeficiency)(int32)ColorBlindMode, (int32)ColorBlindStrength, true, false);
	}
}

int32 UBESettingsShared::GetColorBlindStrength() const
{
	return ColorBlindStrength;
}

void UBESettingsShared::SetColorBlindMode(EColorBlindMode InMode)
{
	if (ColorBlindMode != InMode)
	{
		ColorBlindMode = InMode;
		FSlateApplication::Get().GetRenderer()->SetColorVisionDeficiencyType(
			(EColorVisionDeficiency)(int32)ColorBlindMode, (int32)ColorBlindStrength, true, false);
	}
}

EColorBlindMode UBESettingsShared::GetColorBlindMode() const
{
	return ColorBlindMode;
}


// ===== 字幕設定 =====

void UBESettingsShared::ApplySubtitleOptions()
{
	if (USubtitleDisplaySubsystem* SubtitleSystem = USubtitleDisplaySubsystem::Get(OwningPlayer))
	{
		FSubtitleFormat SubtitleFormat;
		SubtitleFormat.SubtitleTextSize = SubtitleTextSize;
		SubtitleFormat.SubtitleTextColor = SubtitleTextColor;
		SubtitleFormat.SubtitleTextBorder = SubtitleTextBorder;
		SubtitleFormat.SubtitleBackgroundOpacity = SubtitleBackgroundOpacity;

		SubtitleSystem->SetSubtitleDisplayOptions(SubtitleFormat);
	}
}


// ===== 言語設定 =====

void UBESettingsShared::ApplyCultureSettings()
{
	if (bResetToDefaultCulture)
	{
		const FCulturePtr SystemDefaultCulture = FInternationalization::Get().GetDefaultCulture();
		check(SystemDefaultCulture.IsValid());

		const FString CultureToApply = SystemDefaultCulture->GetName();
		if (FInternationalization::Get().SetCurrentCulture(CultureToApply))
		{
			// Clear this string
			GConfig->RemoveKey(TEXT("Internationalization"), TEXT("Culture"), GGameUserSettingsIni);
			GConfig->Flush(false, GGameUserSettingsIni);
		}
		bResetToDefaultCulture = false;
	}
	else if (!PendingCulture.IsEmpty())
	{
		// SetCurrentCulture may trigger PendingCulture to be cleared (if a culture change is broadcast) so we take a copy of it to work with
		const FString CultureToApply = PendingCulture;
		if (FInternationalization::Get().SetCurrentCulture(CultureToApply))
		{
			// Note: This is intentionally saved to the users config
			// We need to localize text before the player logs in and very early in the loading screen
			GConfig->SetString(TEXT("Internationalization"), TEXT("Culture"), *CultureToApply, GGameUserSettingsIni);
			GConfig->Flush(false, GGameUserSettingsIni);
		}
		ClearPendingCulture();
	}
}

void UBESettingsShared::ResetCultureToCurrentSettings()
{
	ClearPendingCulture();
	bResetToDefaultCulture = false;
}

const FString& UBESettingsShared::GetPendingCulture() const
{
	return PendingCulture;
}

void UBESettingsShared::SetPendingCulture(const FString& NewCulture)
{
	PendingCulture = NewCulture;
	bResetToDefaultCulture = false;
	bIsDirty = true;
}

void UBESettingsShared::OnCultureChanged()
{
	ClearPendingCulture();
	bResetToDefaultCulture = false;
}

void UBESettingsShared::ClearPendingCulture()
{
	PendingCulture.Reset();
}

bool UBESettingsShared::IsUsingDefaultCulture() const
{
	FString Culture;
	GConfig->GetString(TEXT("Internationalization"), TEXT("Culture"), Culture, GGameUserSettingsIni);

	return Culture.IsEmpty();
}

void UBESettingsShared::ResetToDefaultCulture()
{
	ClearPendingCulture();
	bResetToDefaultCulture = true;
	bIsDirty = true;
}


// ===== 画面外での音楽の再生設定 =====

void UBESettingsShared::SetAllowAudioInBackgroundSetting(EBEAllowBackgroundAudioSetting NewValue)
{
	if (ChangeValueAndDirty(AllowAudioInBackground, NewValue))
	{
		ApplyAudioSettings();
	}
}

void UBESettingsShared::ApplyAudioSettings()
{
	if (OwningPlayer && OwningPlayer->IsPrimaryPlayer())
	{
		FApp::SetUnfocusedVolumeMultiplier((AllowAudioInBackground != EBEAllowBackgroundAudioSetting::Off) ? 1.0f : 0.0f);
	}
}

void UBESettingsShared::ApplyInputSensitivity()
{

}


////////////////////////////////////////
//	UBESettingsShared : 操作

void UBESettingsShared::SaveSettings()
{
	check(OwningPlayer);
	UGameplayStatics::SaveGameToSlot(this, SHARED_SETTINGS_SLOT_NAME, OwningPlayer->GetLocalPlayerIndex());
}

void UBESettingsShared::ApplySettings()
{
	ApplySubtitleOptions();
	ApplyAudioSettings();
	ApplyCultureSettings();

	OnSettingApplied.Broadcast(this);
}


////////////////////////////////////////
//	UBESettingsShared : ユーティリティ

UBESettingsShared* UBESettingsShared::Get(const ULocalPlayer* LocalPlayer)
{
	if (const UBELocalPlayer* BELP = Cast<UBELocalPlayer>(LocalPlayer))
	{
		return BELP->GetSharedSettings();
	}

	return nullptr;
}

UBESettingsShared* UBESettingsShared::LoadOrCreateSettings(const ULocalPlayer* LocalPlayer)
{
	UBESettingsShared* SharedSettings = nullptr;

	// If the save game exists, load it.
	if (UGameplayStatics::DoesSaveGameExist(SHARED_SETTINGS_SLOT_NAME, LocalPlayer->GetLocalPlayerIndex()))
	{
		USaveGame* Slot = UGameplayStatics::LoadGameFromSlot(SHARED_SETTINGS_SLOT_NAME, LocalPlayer->GetLocalPlayerIndex());
		SharedSettings = Cast<UBESettingsShared>(Slot);
	}
	
	if (SharedSettings == nullptr)
	{
		SharedSettings = Cast<UBESettingsShared>(UGameplayStatics::CreateSaveGameObject(UBESettingsShared::StaticClass()));
	}

	SharedSettings->Initialize(const_cast<ULocalPlayer*>(LocalPlayer));
	SharedSettings->ApplySettings();

	return SharedSettings;
}

#undef LOCTEXT_NAMESPACE
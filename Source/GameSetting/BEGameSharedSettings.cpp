// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#include "BEGameSharedSettings.h"

#include "Player/BELocalPlayer.h"
#include "Development/BEDeveloperGameSettings.h"

#include "Framework/Application/SlateApplication.h"
#include "Internationalization/Culture.h"
#include "Internationalization/Internationalization.h"
#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEGameSharedSettings)

const FString UBEGameSharedSettings::NAME_SharedSettings_SaveSlot("GameSettings");


void UBEGameSharedSettings::Initialize(ULocalPlayer* LocalPlayer)
{
	check(LocalPlayer);
	
	OwningPlayer = Cast<UBELocalPlayer>(LocalPlayer);
	OnInitialized(LocalPlayer);
}


void UBEGameSharedSettings::SaveSettings()
{
	check(OwningPlayer);

	UGameplayStatics::SaveGameToSlot(this, NAME_SharedSettings_SaveSlot, OwningPlayer->GetLocalPlayerIndex());
}

void UBEGameSharedSettings::ApplySettings()
{
	OnSettingApplied.Broadcast(this);
}


UBEGameSharedSettings* UBEGameSharedSettings::LoadOrCreateSettings(const ULocalPlayer* LocalPlayer)
{
	// 既に保存されたデータがある場合はそれを読み込む
	if (UGameplayStatics::DoesSaveGameExist(NAME_SharedSettings_SaveSlot, LocalPlayer->GetLocalPlayerIndex()))
	{
		USaveGame* Slot = UGameplayStatics::LoadGameFromSlot(NAME_SharedSettings_SaveSlot, LocalPlayer->GetLocalPlayerIndex());
		return Cast<UBEGameSharedSettings>(Slot);
	}

	// 既に保存されたデータがない場合は作成する
	const UBEDeveloperGameSettings* DevSettings = GetDefault<UBEDeveloperGameSettings>();
	check(DevSettings);

	FSoftClassPath SettingClassPath = DevSettings->SharedSettingClass;
	if (SettingClassPath.IsValid())
	{
		UClass* SettingClass = SettingClassPath.ResolveClass();
		if (!SettingClass)
		{
			SettingClass = SettingClassPath.TryLoadClass<UClass>();
		}

		return Cast<UBEGameSharedSettings>(UGameplayStatics::CreateSaveGameObject(SettingClass));
	}
	else
	{
		return Cast<UBEGameSharedSettings>(UGameplayStatics::CreateSaveGameObject(UBEGameSharedSettings::StaticClass()));
	}
}


void UBEGameSharedSettings::SetPendingCulture(const FString& NewCulture)
{
	PendingCulture = NewCulture;
	bResetToDefaultCulture = false;
	bIsDirty = true;
}

void UBEGameSharedSettings::OnCultureChanged()
{
	ClearPendingCulture();
	bResetToDefaultCulture = false;
}

void UBEGameSharedSettings::ClearPendingCulture()
{
	PendingCulture.Reset();
}

bool UBEGameSharedSettings::IsUsingDefaultCulture() const
{
	FString Culture;
	GConfig->GetString(TEXT("Internationalization"), TEXT("Culture"), Culture, GGameUserSettingsIni);

	return Culture.IsEmpty();
}

void UBEGameSharedSettings::ResetToDefaultCulture()
{
	ClearPendingCulture();
	bResetToDefaultCulture = true;
	bIsDirty = true;
}

void UBEGameSharedSettings::ApplyCultureSettings()
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

void UBEGameSharedSettings::ResetCultureToCurrentSettings()
{
	ClearPendingCulture();
	bResetToDefaultCulture = false;
}


void UBEGameSharedSettings::SetColorBlindMode(EColorBlindMode InMode)
{
	if (ColorBlindMode != InMode)
	{
		ColorBlindMode = InMode;
		FSlateApplication::Get().GetRenderer()->SetColorVisionDeficiencyType(
			(EColorVisionDeficiency)(int32)ColorBlindMode, (int32)ColorBlindStrength, true, false);
	}
}

void UBEGameSharedSettings::SetColorBlindStrength(int32 InColorBlindStrength)
{
	InColorBlindStrength = FMath::Clamp(InColorBlindStrength, 0, 10);
	if (ColorBlindStrength != InColorBlindStrength)
	{
		ColorBlindStrength = InColorBlindStrength;
		FSlateApplication::Get().GetRenderer()->SetColorVisionDeficiencyType(
			(EColorVisionDeficiency)(int32)ColorBlindMode, (int32)ColorBlindStrength, true, false);
	}
}


void UBEGameSharedSettings::ApplySubtitleOptions()
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

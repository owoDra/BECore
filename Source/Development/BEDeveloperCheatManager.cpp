// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#include "BEDeveloperCheatManager.h"

#include "Player/BEPlayerController.h"
#include "Camera/BEDebugCameraController.h"
#include "Ability/BEAbilitySystemComponent.h"
#include "System/BEAssetManager.h"
#include "Character/Component/BEPawnHealthComponent.h"
#include "Character/Component/BEPawnBasicComponent.h"
#include "System/BESystemStatics.h"
#include "System/BEGameData.h"
#include "Development/BEDeveloperCheatSettings.h"
#include "GameplayTag/BETags_InitState.h"
#include "GameplayTag/BETags_GameplayEffect.h"
#include "GameplayTag/BETags_Flag.h"
#include "BELogChannels.h"

#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "Engine/Console.h"
#include "AbilitySystemGlobals.h"
#include "GameFramework/HUD.h"
#include "GameplayEffect.h"
#include "GameplayTagsManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEDeveloperCheatManager)

/////////////////////////////////////////////////

DEFINE_LOG_CATEGORY(LogBECheat);

/////////////////////////////////////////////////

namespace BECheat
{
	static const FName NAME_Fixed = FName(TEXT("Fixed"));
	
	static bool bEnableDebugCameraCycling = false;
	static FAutoConsoleVariableRef CVarEnableDebugCameraCycling(
		TEXT("BECheat.EnableDebugCameraCycling"),
		bEnableDebugCameraCycling,
		TEXT("If true then you can cycle the debug camera while running the game."),
		ECVF_Cheat);

	static bool bStartInGodMode = false;
	static FAutoConsoleVariableRef CVarStartInGodMode(
		TEXT("BECheat.StartInGodMode"),
		bStartInGodMode,
		TEXT("If true then the God cheat will be applied on begin play"),
		ECVF_Cheat);
};

/////////////////////////////////////////////////

UBEDeveloperCheatManager::UBEDeveloperCheatManager()
{
	DebugCameraControllerClass = ABEDebugCameraController::StaticClass();
}

void UBEDeveloperCheatManager::InitCheatManager()
{
	Super::InitCheatManager();

#if WITH_EDITOR
	if (GIsEditor)
	{
		APlayerController* PC = GetOuterAPlayerController();
		for (const FBECheatToRun& CheatRow : GetDefault<UBEDeveloperCheatSettings>()->CheatsToRun)
		{
			if (CheatRow.Phase == ECheatExecutionTime::OnCheatManagerCreated)
			{
				PC->ConsoleCommand(CheatRow.Cheat, /*bWriteToLog=*/ true);
			}
		}
	}
#endif

	if (BECheat::bStartInGodMode)
	{
		God();	
	}
}


void UBEDeveloperCheatManager::Cheat(const FString& Msg)
{
	if (ABEPlayerController* BEPC = Cast<ABEPlayerController>(GetOuterAPlayerController()))
	{
		BEPC->ServerCheat(Msg.Left(128));
	}
}

void UBEDeveloperCheatManager::CheatAll(const FString& Msg)
{
	if (ABEPlayerController* BEPC = Cast<ABEPlayerController>(GetOuterAPlayerController()))
	{
		BEPC->ServerCheatAll(Msg.Left(128));
	}
}

void UBEDeveloperCheatManager::PlayNextGame()
{
	UBESystemStatics::PlayNextGame(this);
}

void UBEDeveloperCheatManager::ToggleFixedCamera()
{
	if (InFixedCamera())
	{
		DisableFixedCamera();
	}
	else
	{
		EnableFixedCamera();
	}
}

void UBEDeveloperCheatManager::CycleDebugCameras()
{
	if (!BECheat::bEnableDebugCameraCycling)
	{
		return;
	}
	
	if (InDebugCamera())
	{
		EnableFixedCamera();
		DisableDebugCamera();
	}
	else if (InFixedCamera())
	{
		DisableFixedCamera();
		DisableDebugCamera();
	}
	else
	{
		EnableDebugCamera();
		DisableFixedCamera();
	}
}

void UBEDeveloperCheatManager::CycleAbilitySystemDebug()
{
	APlayerController* PC = Cast<APlayerController>(GetOuterAPlayerController());

	if (PC && PC->MyHUD)
	{
		if (!PC->MyHUD->bShowDebugInfo || !PC->MyHUD->DebugDisplay.Contains(TEXT("AbilitySystem")))
		{
			PC->MyHUD->ShowDebug(TEXT("AbilitySystem"));
		}

		PC->ConsoleCommand(TEXT("AbilitySystem.Debug.NextCategory"));
	}
}

void UBEDeveloperCheatManager::CancelActivatedAbilities()
{
	if (UBEAbilitySystemComponent* BEASC = GetPlayerAbilitySystemComponent())
	{
		const bool bReplicateCancelAbility = true;
		BEASC->CancelInputActivatedAbilities(bReplicateCancelAbility);
	}
}

void UBEDeveloperCheatManager::AddTagToSelf(FString TagName)
{
	FGameplayTag Tag = FindTagByString(TagName, true);
	if (Tag.IsValid())
	{
		if (UBEAbilitySystemComponent* BEASC = GetPlayerAbilitySystemComponent())
		{
			BEASC->AddDynamicTagGameplayEffect(Tag);
		}
	}
	else
	{
		UE_LOG(LogBECheat, Display, TEXT("AddTagToSelf: Could not find any tag matching [%s]."), *TagName);
	}
}

void UBEDeveloperCheatManager::RemoveTagFromSelf(FString TagName)
{
	FGameplayTag Tag = FindTagByString(TagName, true);
	if (Tag.IsValid())
	{
		if (UBEAbilitySystemComponent* BEASC = GetPlayerAbilitySystemComponent())
		{
			BEASC->RemoveDynamicTagGameplayEffect(Tag);
		}
	}
	else
	{
		UE_LOG(LogBECheat, Display, TEXT("RemoveTagFromSelf: Could not find any tag matching [%s]."), *TagName);
	}
}

void UBEDeveloperCheatManager::DamageSelf(float DamageAmount)
{
	if (UBEAbilitySystemComponent* BEASC = GetPlayerAbilitySystemComponent())
	{
		ApplySetByCallerDamage(BEASC, DamageAmount);
	}
}

void UBEDeveloperCheatManager::DamageTarget(float DamageAmount)
{
	if (ABEPlayerController* BEPC = Cast<ABEPlayerController>(GetOuterAPlayerController()))
	{
		if (BEPC->GetNetMode() == NM_Client)
		{
			// Automatically send cheat to server for convenience.
			BEPC->ServerCheat(FString::Printf(TEXT("DamageTarget %.2f"), DamageAmount));
			return;
		}

		FHitResult TargetHitResult;
		AActor* TargetActor = GetTarget(BEPC, TargetHitResult);

		if (UBEAbilitySystemComponent* BETargetASC = Cast<UBEAbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor)))
		{
			ApplySetByCallerDamage(BETargetASC, DamageAmount);
		}
	}
}

void UBEDeveloperCheatManager::HealHealthSelf(float HealAmount)
{
	if (UBEAbilitySystemComponent* BEASC = GetPlayerAbilitySystemComponent())
	{
		ApplySetByCallerHealHealth(BEASC, HealAmount);
	}
}

void UBEDeveloperCheatManager::HealHealthTarget(float HealAmount)
{
	if (ABEPlayerController* BEPC = Cast<ABEPlayerController>(GetOuterAPlayerController()))
	{
		FHitResult TargetHitResult;
		AActor* TargetActor = GetTarget(BEPC, TargetHitResult);

		if (UBEAbilitySystemComponent* BETargetASC = Cast<UBEAbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor)))
		{
			ApplySetByCallerHealHealth(BETargetASC, HealAmount);
		}
	}
}

void UBEDeveloperCheatManager::HealShieldSelf(float HealAmount)
{
	if (UBEAbilitySystemComponent* BEASC = GetPlayerAbilitySystemComponent())
	{
		ApplySetByCallerHealShield(BEASC, HealAmount);
	}
}

void UBEDeveloperCheatManager::HealShieldTarget(float HealAmount)
{
	if (ABEPlayerController* BEPC = Cast<ABEPlayerController>(GetOuterAPlayerController()))
	{
		FHitResult TargetHitResult;
		AActor* TargetActor = GetTarget(BEPC, TargetHitResult);

		if (UBEAbilitySystemComponent* BETargetASC = Cast<UBEAbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor)))
		{
			ApplySetByCallerHealShield(BETargetASC, HealAmount);
		}
	}
}

void UBEDeveloperCheatManager::DamageSelfDestruct()
{
	if (ABEPlayerController* BEPC = Cast<ABEPlayerController>(GetOuterAPlayerController()))
	{
		if (const UBEPawnBasicComponent* CharacterBasic = UBEPawnBasicComponent::FindPawnBasicComponent(BEPC->GetPawn()))
		{
			if (CharacterBasic->HasReachedInitState(TAG_InitState_GameplayReady))
			{
				if (UBEPawnHealthComponent* HealthComponent = UBEPawnHealthComponent::FindPawnHealthComponent(BEPC->GetPawn()))
				{
					HealthComponent->DamageSelfDestruct();
				}
			}
		}
	}
}

void UBEDeveloperCheatManager::God()
{
	if (ABEPlayerController* BEPC = Cast<ABEPlayerController>(GetOuterAPlayerController()))
	{
		if (BEPC->GetNetMode() == NM_Client)
		{
			// Automatically send cheat to server for convenience.
			BEPC->ServerCheat(FString::Printf(TEXT("God")));
			return;
		}
	}

	UnlimitedHealth();
	UnlimitedCost();
}

void UBEDeveloperCheatManager::UnlimitedHealth(int32 Enabled)
{
	if (UBEAbilitySystemComponent* BEASC = GetPlayerAbilitySystemComponent())
	{
		const FGameplayTag Tag = TAG_Flag_UnlimitedHealth;
		const bool bHasTag = BEASC->HasMatchingGameplayTag(Tag);

		if ((Enabled == -1) || ((Enabled > 0) && !bHasTag) || ((Enabled == 0) && bHasTag))
		{
			if (bHasTag)
			{
				BEASC->RemoveDynamicTagGameplayEffect(Tag);
			}
			else
			{
				BEASC->AddDynamicTagGameplayEffect(Tag);
			}
		}
	}
}

void UBEDeveloperCheatManager::UnlimitedCost(int32 Enabled)
{
	if (UBEAbilitySystemComponent* BEASC = GetPlayerAbilitySystemComponent())
	{
		const FGameplayTag Tag = TAG_Flag_UnlimitedCost;
		const bool bHasTag = BEASC->HasMatchingGameplayTag(Tag);

		if ((Enabled == -1) || ((Enabled > 0) && !bHasTag) || ((Enabled == 0) && bHasTag))
		{
			if (bHasTag)
			{
				BEASC->RemoveDynamicTagGameplayEffect(Tag);
			}
			else
			{
				BEASC->AddDynamicTagGameplayEffect(Tag);
			}
		}
	}
}


void UBEDeveloperCheatManager::CheatOutputText(const FString& TextToOutput)
{
#if USING_CHEAT_MANAGER
	// Output to the console.
	if (GEngine && GEngine->GameViewport && GEngine->GameViewport->ViewportConsole)
	{
		GEngine->GameViewport->ViewportConsole->OutputText(TextToOutput);
	}

	// Output to log.
	UE_LOG(LogBECheat, Display, TEXT("%s"), *TextToOutput);
#endif // USING_CHEAT_MANAGER
}

FGameplayTag UBEDeveloperCheatManager::FindTagByString(FString TagString, bool bMatchPartialString)
{
	const UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
	FGameplayTag Tag = Manager.RequestGameplayTag(FName(*TagString), false);

	if (!Tag.IsValid() && bMatchPartialString)
	{
		FGameplayTagContainer AllTags;
		Manager.RequestAllGameplayTags(AllTags, true);

		for (const FGameplayTag TestTag : AllTags)
		{
			if (TestTag.ToString().Contains(TagString))
			{
				UE_LOG(LogBE, Display, TEXT("Could not find exact match for tag [%s] but found partial match on tag [%s]."), *TagString, *TestTag.ToString());
				Tag = TestTag;
				break;
			}
		}
	}

	return Tag;
}


void UBEDeveloperCheatManager::EnableDebugCamera()
{
	Super::EnableDebugCamera();
}

void UBEDeveloperCheatManager::DisableDebugCamera()
{
	FVector DebugCameraLocation;
	FRotator DebugCameraRotation;

	ADebugCameraController* DebugCC = Cast<ADebugCameraController>(GetOuter());
	APlayerController* OriginalPC = nullptr;

	if (DebugCC)
	{
		OriginalPC = DebugCC->OriginalControllerRef;
		DebugCC->GetPlayerViewPoint(DebugCameraLocation, DebugCameraRotation);
	}

	Super::DisableDebugCamera();

	if (OriginalPC && OriginalPC->PlayerCameraManager && (OriginalPC->PlayerCameraManager->CameraStyle == BECheat::NAME_Fixed))
	{
		OriginalPC->SetInitialLocationAndRotation(DebugCameraLocation, DebugCameraRotation);

		OriginalPC->PlayerCameraManager->ViewTarget.POV.Location = DebugCameraLocation;
		OriginalPC->PlayerCameraManager->ViewTarget.POV.Rotation = DebugCameraRotation;
		OriginalPC->PlayerCameraManager->PendingViewTarget.POV.Location = DebugCameraLocation;
		OriginalPC->PlayerCameraManager->PendingViewTarget.POV.Rotation = DebugCameraRotation;
	}
}

bool UBEDeveloperCheatManager::InDebugCamera() const
{
	return (Cast<ADebugCameraController>(GetOuter()) ? true : false);
}

void UBEDeveloperCheatManager::EnableFixedCamera()
{
	const ADebugCameraController* DebugCC = Cast<ADebugCameraController>(GetOuter());
	APlayerController* PC = (DebugCC ? ToRawPtr(DebugCC->OriginalControllerRef) : GetOuterAPlayerController());

	if (PC && PC->PlayerCameraManager)
	{
		PC->SetCameraMode(BECheat::NAME_Fixed);
	}
}

void UBEDeveloperCheatManager::DisableFixedCamera()
{
	const ADebugCameraController* DebugCC = Cast<ADebugCameraController>(GetOuter());
	APlayerController* PC = (DebugCC ? ToRawPtr(DebugCC->OriginalControllerRef) : GetOuterAPlayerController());

	if (PC && PC->PlayerCameraManager)
	{
		PC->SetCameraMode(NAME_Default);
	}
}

bool UBEDeveloperCheatManager::InFixedCamera() const
{
	const ADebugCameraController* DebugCC = Cast<ADebugCameraController>(GetOuter());
	const APlayerController* PC = (DebugCC ? ToRawPtr(DebugCC->OriginalControllerRef) : GetOuterAPlayerController());

	if (PC && PC->PlayerCameraManager)
	{
		return (PC->PlayerCameraManager->CameraStyle == BECheat::NAME_Fixed);
	}

	return false;
}


void UBEDeveloperCheatManager::ApplySetByCallerDamage(UBEAbilitySystemComponent* BEASC, float DamageAmount)
{
	check(BEASC);

	TSubclassOf<UGameplayEffect> DamageGE = UBEAssetManager::GetSubclass(UBEGameData::Get().DamageGameplayEffect_SetByCaller);
	FGameplayEffectSpecHandle SpecHandle = BEASC->MakeOutgoingSpec(DamageGE, 1.0f, BEASC->MakeEffectContext());

	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(TAG_SetByCaller_Damage, DamageAmount);
		BEASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

void UBEDeveloperCheatManager::ApplySetByCallerHealHealth(UBEAbilitySystemComponent* BEASC, float HealAmount)
{
	check(BEASC);

	TSubclassOf<UGameplayEffect> HealGE = UBEAssetManager::GetSubclass(UBEGameData::Get().HealHealthGameplayEffect_SetByCaller);
	FGameplayEffectSpecHandle SpecHandle = BEASC->MakeOutgoingSpec(HealGE, 1.0f, BEASC->MakeEffectContext());

	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(TAG_SetByCaller_Heal_Health, HealAmount);
		BEASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

void UBEDeveloperCheatManager::ApplySetByCallerHealShield(UBEAbilitySystemComponent* BEASC, float HealAmount)
{
	check(BEASC);

	TSubclassOf<UGameplayEffect> HealGE = UBEAssetManager::GetSubclass(UBEGameData::Get().HealShieldGameplayEffect_SetByCaller);
	FGameplayEffectSpecHandle SpecHandle = BEASC->MakeOutgoingSpec(HealGE, 1.0f, BEASC->MakeEffectContext());

	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(TAG_SetByCaller_Heal_Shield, HealAmount);
		BEASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

UBEAbilitySystemComponent* UBEDeveloperCheatManager::GetPlayerAbilitySystemComponent() const
{
	if (ABEPlayerController* BEPC = Cast<ABEPlayerController>(GetOuterAPlayerController()))
	{
		return BEPC->GetBEAbilitySystemComponent();
	}
	return nullptr;
}
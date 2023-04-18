// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BECheatManager.h"

#include "BEPlayerController.h"
#include "BEDebugCameraController.h"
#include "Ability/BEAbilitySystemComponent.h"
#include "System/BEAssetManager.h"
#include "Character/BEHealthComponent.h"
#include "Character/BEPawnExtensionComponent.h"
#include "System/BESystemStatics.h"
#include "System/BEGameData.h"
#include "Development/BEDeveloperSettings.h"
#include "BEGameplayTags.h"

#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "Engine/Console.h"
#include "GameFramework/HUD.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffect.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BECheatManager)

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

UBECheatManager::UBECheatManager()
{
	DebugCameraControllerClass = ABEDebugCameraController::StaticClass();
}

void UBECheatManager::InitCheatManager()
{
	Super::InitCheatManager();

#if WITH_EDITOR
	if (GIsEditor)
	{
		APlayerController* PC = GetOuterAPlayerController();
		for (const FBECheatToRun& CheatRow : GetDefault<UBEDeveloperSettings>()->CheatsToRun)
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


void UBECheatManager::Cheat(const FString& Msg)
{
	if (ABEPlayerController* BEPC = Cast<ABEPlayerController>(GetOuterAPlayerController()))
	{
		BEPC->ServerCheat(Msg.Left(128));
	}
}

void UBECheatManager::CheatAll(const FString& Msg)
{
	if (ABEPlayerController* BEPC = Cast<ABEPlayerController>(GetOuterAPlayerController()))
	{
		BEPC->ServerCheatAll(Msg.Left(128));
	}
}

void UBECheatManager::PlayNextGame()
{
	UBESystemStatics::PlayNextGame(this);
}

void UBECheatManager::ToggleFixedCamera()
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

void UBECheatManager::CycleDebugCameras()
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

void UBECheatManager::CycleAbilitySystemDebug()
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

void UBECheatManager::CancelActivatedAbilities()
{
	if (UBEAbilitySystemComponent* BEASC = GetPlayerAbilitySystemComponent())
	{
		const bool bReplicateCancelAbility = true;
		BEASC->CancelInputActivatedAbilities(bReplicateCancelAbility);
	}
}

void UBECheatManager::AddTagToSelf(FString TagName)
{
	FGameplayTag Tag = FBEGameplayTags::FindTagByString(TagName, true);
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

void UBECheatManager::RemoveTagFromSelf(FString TagName)
{
	FGameplayTag Tag = FBEGameplayTags::FindTagByString(TagName, true);
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

void UBECheatManager::DamageSelf(float DamageAmount)
{
	if (UBEAbilitySystemComponent* BEASC = GetPlayerAbilitySystemComponent())
	{
		ApplySetByCallerDamage(BEASC, DamageAmount);
	}
}

void UBECheatManager::DamageTarget(float DamageAmount)
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

void UBECheatManager::HealHealthSelf(float HealAmount)
{
	if (UBEAbilitySystemComponent* BEASC = GetPlayerAbilitySystemComponent())
	{
		ApplySetByCallerHealHealth(BEASC, HealAmount);
	}
}

void UBECheatManager::HealHealthTarget(float HealAmount)
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

void UBECheatManager::HealShieldSelf(float HealAmount)
{
	if (UBEAbilitySystemComponent* BEASC = GetPlayerAbilitySystemComponent())
	{
		ApplySetByCallerHealShield(BEASC, HealAmount);
	}
}

void UBECheatManager::HealShieldTarget(float HealAmount)
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

void UBECheatManager::DamageSelfDestruct()
{
	if (ABEPlayerController* BEPC = Cast<ABEPlayerController>(GetOuterAPlayerController()))
	{
		if (const UBEPawnExtensionComponent* PawnExtComp = UBEPawnExtensionComponent::FindPawnExtensionComponent(BEPC->GetPawn()))
		{
			if (PawnExtComp->HasReachedInitState(TAG_InitState_GameplayReady))
			{
				if (UBEHealthComponent* HealthComponent = UBEHealthComponent::FindHealthComponent(BEPC->GetPawn()))
				{
					HealthComponent->DamageSelfDestruct();
				}
			}
		}
	}
}

void UBECheatManager::God()
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
	UnlimitedAmmo();
	UnlimitedAbility();
}

void UBECheatManager::UnlimitedHealth(int32 Enabled)
{
	if (UBEAbilitySystemComponent* BEASC = GetPlayerAbilitySystemComponent())
	{
		const FGameplayTag Tag = TAG_Cheat_UnlimitedHealth;
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

void UBECheatManager::UnlimitedAmmo(int32 Enabled)
{
	if (UBEAbilitySystemComponent* BEASC = GetPlayerAbilitySystemComponent())
	{
		const FGameplayTag Tag = TAG_Cheat_UnlimitedAmmo;
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

void UBECheatManager::UnlimitedAbility(int32 Enabled)
{
	if (UBEAbilitySystemComponent* BEASC = GetPlayerAbilitySystemComponent())
	{
		const FGameplayTag Tag = TAG_Cheat_UnlimitedAbility;
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


void UBECheatManager::CheatOutputText(const FString& TextToOutput)
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

void UBECheatManager::EnableDebugCamera()
{
	Super::EnableDebugCamera();
}

void UBECheatManager::DisableDebugCamera()
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

bool UBECheatManager::InDebugCamera() const
{
	return (Cast<ADebugCameraController>(GetOuter()) ? true : false);
}

void UBECheatManager::EnableFixedCamera()
{
	const ADebugCameraController* DebugCC = Cast<ADebugCameraController>(GetOuter());
	APlayerController* PC = (DebugCC ? ToRawPtr(DebugCC->OriginalControllerRef) : GetOuterAPlayerController());

	if (PC && PC->PlayerCameraManager)
	{
		PC->SetCameraMode(BECheat::NAME_Fixed);
	}
}

void UBECheatManager::DisableFixedCamera()
{
	const ADebugCameraController* DebugCC = Cast<ADebugCameraController>(GetOuter());
	APlayerController* PC = (DebugCC ? ToRawPtr(DebugCC->OriginalControllerRef) : GetOuterAPlayerController());

	if (PC && PC->PlayerCameraManager)
	{
		PC->SetCameraMode(NAME_Default);
	}
}

bool UBECheatManager::InFixedCamera() const
{
	const ADebugCameraController* DebugCC = Cast<ADebugCameraController>(GetOuter());
	const APlayerController* PC = (DebugCC ? ToRawPtr(DebugCC->OriginalControllerRef) : GetOuterAPlayerController());

	if (PC && PC->PlayerCameraManager)
	{
		return (PC->PlayerCameraManager->CameraStyle == BECheat::NAME_Fixed);
	}

	return false;
}

void UBECheatManager::ApplySetByCallerDamage(UBEAbilitySystemComponent* BEASC, float DamageAmount)
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

void UBECheatManager::ApplySetByCallerHealHealth(UBEAbilitySystemComponent* BEASC, float HealAmount)
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

void UBECheatManager::ApplySetByCallerHealShield(UBEAbilitySystemComponent* BEASC, float HealAmount)
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

UBEAbilitySystemComponent* UBECheatManager::GetPlayerAbilitySystemComponent() const
{
	if (ABEPlayerController* BEPC = Cast<ABEPlayerController>(GetOuterAPlayerController()))
	{
		return BEPC->GetBEAbilitySystemComponent();
	}
	return nullptr;
}
// Copyright Eigi Chin

#pragma once

#include "CoreMinimal.h"
#include "Logging/LogMacros.h"

BECORE_API DECLARE_LOG_CATEGORY_EXTERN(LogBE, Log, All);
BECORE_API DECLARE_LOG_CATEGORY_EXTERN(LogBEExperience, Log, All);
BECORE_API DECLARE_LOG_CATEGORY_EXTERN(LogBEMovement, Log, All);
BECORE_API DECLARE_LOG_CATEGORY_EXTERN(LogBEAbilitySystem, Log, All);
BECORE_API DECLARE_LOG_CATEGORY_EXTERN(LogBEEquipmentSystem, Log, All);
BECORE_API DECLARE_LOG_CATEGORY_EXTERN(LogBEInventorySystem, Log, All);
BECORE_API DECLARE_LOG_CATEGORY_EXTERN(LogBEWeaponSystem, Log, All);
BECORE_API DECLARE_LOG_CATEGORY_EXTERN(LogBETeams, Log, All);

BECORE_API FString GetClientServerContextString(UObject* ContextObject = nullptr);

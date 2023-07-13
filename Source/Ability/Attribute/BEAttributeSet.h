// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#pragma once

#include "AttributeSet.h"

#include "Delegates/Delegate.h"
#include "HAL/PlatformMisc.h"
#include "UObject/UObjectGlobals.h"

#include "BEAttributeSet.generated.h"

class AActor;
class UObject;
class UWorld;
class UBEAbilitySystemComponent;
struct FGameplayEffectSpec;


/**
 * This macro defines a set of helper functions for accessing and initializing attributes.
 *
 * The following example of the macro:
 *		ATTRIBUTE_ACCESSORS(UBEHealthSet, Health)
 * will create the following functions:
 *		static FGameplayAttribute GetHealthAttribute();
 *		float GetHealth() const;
 *		void SetHealth(float NewVal);
 *		void InitHealth(float NewVal);
 */
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)


// Delegate used to broadcast attribute events.
DECLARE_MULTICAST_DELEGATE_FourParams(FBEAttributeEvent, AActor* /*EffectInstigator*/, AActor* /*EffectCauser*/, const FGameplayEffectSpec& /*EffectSpec*/, float /*EffectMagnitude*/);


/**
 * UBEAttributeSet
 *
 *	このプロジェクトにおける基本のAttribute Set
 */
UCLASS()
class BECORE_API UBEAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:

	UBEAttributeSet();

	UWorld* GetWorld() const override;

	UBEAbilitySystemComponent* GetBEAbilitySystemComponent() const;
};

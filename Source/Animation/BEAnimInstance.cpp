// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BEAnimInstance.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Character/BECharacter.h"
#include "Character/BECharacterMovementComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEAnimInstance)


UBEAnimInstance::UBEAnimInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UBEAnimInstance::InitializeWithAbilitySystem(UAbilitySystemComponent* ASC)
{
	check(ASC);

	GameplayTagPropertyMap.Initialize(this, ASC);
}

#if WITH_EDITOR
EDataValidationResult UBEAnimInstance::IsDataValid(TArray<FText>& ValidationErrors)
{
	Super::IsDataValid(ValidationErrors);

	GameplayTagPropertyMap.IsDataValid(this, ValidationErrors);

	return ((ValidationErrors.Num() > 0) ? EDataValidationResult::Invalid : EDataValidationResult::Valid);
}
#endif // WITH_EDITOR

void UBEAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	if (AActor* OwningActor = GetOwningActor())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningActor))
		{
			InitializeWithAbilitySystem(ASC);
		}
	}
}

void UBEAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	const ABECharacter* Character = Cast<ABECharacter>(GetOwningActor());
	if (!Character)
	{
		return;
	}

	UBECharacterMovementComponent* CharMoveComp = CastChecked<UBECharacterMovementComponent>(Character->GetCharacterMovement());
	const FBECharacterGroundInfo& GroundInfo = CharMoveComp->GetGroundInfo();
	GroundDistance = GroundInfo.GroundDistance;
}

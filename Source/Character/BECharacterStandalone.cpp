// Copyright Eigi Chin

#include "BECharacterStandalone.h"

#include "Ability/BEAbilitySystemComponent.h"
#include "Character/Component/BEPawnBasicComponent.h"
#include "Character/BEPawnData.h"

#include "AbilitySystemComponent.h"
#include "HAL/Platform.h"
#include "Misc/AssertionMacros.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BECharacterStandalone)


ABECharacterStandalone::ABECharacterStandalone(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UBEAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// 安定して AbilitySystemComponent を利用するために
	// ネットワークの更新頻度を高く設定
	NetUpdateFrequency = 100.0f;
}

void ABECharacterStandalone::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	check(AbilitySystemComponent);
	check(CharacterBasicComponent);
	CharacterBasicComponent->InitializeAbilitySystem(AbilitySystemComponent, this);
}

void ABECharacterStandalone::OnPawnBasicInitialized()
{
	UBEAbilitySystemComponent* BEASC = CharacterBasicComponent->GetBEAbilitySystemComponent();
	check(BEASC);

	const UBEPawnData* PawnData = CharacterBasicComponent->GetPawnData();
	check(PawnData);

	for (const UBEAbilitySet* AbilitySet : PawnData->AbilitySets)
	{
		if (AbilitySet)
		{
			AbilitySet->GiveToAbilitySystem(BEASC, &PawnDataAbilityHandles);
		}
	}
}

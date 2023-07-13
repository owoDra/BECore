// Copyright owoDra

#pragma once

#include "Character/BECharacter.h"

#include "Ability/BEAbilitySet.h"

#include "UObject/UObjectGlobals.h"

#include "BECharacterStandalone.generated.h"

class UAbilitySystemComponent;
class UBEAbilitySystemComponent;
class UObject;
struct FBEAbilitySet_GrantedHandles;


/**
 * ABECharacterStandalone
 *
 *	Character 自体で AbilitySystem を保有している BECharacter
 *  単純な敵 NPC などといった PlayerState を保有しない非プレイヤー(プレイヤーボット)に使用する
 */
UCLASS(Blueprintable)
class BECORE_API ABECharacterStandalone : public ABECharacter
{
	GENERATED_BODY()

public:
	ABECharacterStandalone(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;

protected:
	virtual void OnPawnBasicInitialized() override;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBEAbilitySystemComponent> AbilitySystemComponent;

	FBEAbilitySet_GrantedHandles PawnDataAbilityHandles;
};

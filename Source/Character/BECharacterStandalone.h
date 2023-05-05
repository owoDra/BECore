// Copyright Eigi Chin

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
 *	Character ���̂� AbilitySystem ��ۗL���Ă��� BECharacter
 *  �P���ȓG NPC �ȂǂƂ����� PlayerState ��ۗL���Ȃ���v���C���[(�v���C���[�{�b�g)�Ɏg�p����
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

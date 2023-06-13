// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameplayEffectTypes.h"
#include "BEAnimInstance.generated.h"

class UAbilitySystemComponent;


/**
 * UBEAnimInstance
 *
 *	Pawn または Character の見た目となる Mesh に適応するメインの AnimInsntace。
 *  CopyPose や AnimLayer などには使用せず Animation の主要な処理もおこなう。
 */
UCLASS(Config = Game)
class UBEAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	UBEAnimInstance(const FObjectInitializer& ObjectInitializer);

protected:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
#endif // WITH_EDITOR

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

public:
	/**
	 * InitializeWithAbilitySystem
	 * 
	 *  AbilitySystem との初期化を行う。
	 *  GameplayTag との紐づけなどを行う。
	 */
	virtual void InitializeWithAbilitySystem(UAbilitySystemComponent* ASC);

protected:
	// ブループリント変数にマッピングできる GameplayTag。
	// タグが追加または削除された時に変数を自動的に更新する。
	// これらは GameplayTag を手動でクエリする代わりに使用する。
	UPROPERTY(EditDefaultsOnly, Category = "GameplayTags")
	FGameplayTagBlueprintPropertyMap GameplayTagPropertyMap;

	UPROPERTY(BlueprintReadOnly, Category = "Character State Data")
	float GroundDistance = -1.0f;
};

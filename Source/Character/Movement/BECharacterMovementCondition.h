// Copyright owoDra

#pragma once

#include "UObject/UObjectGlobals.h"

#include "UObject/ObjectPtr.h"

#include "BECharacterMovementCondition.generated.h"

class UBECharacterMovementComponent;


/**
 * UBECharacterMovementCondition
 *
 *  BECharacterMovementComponent で使用する特定の State に遷移可能かを判定する
 */
UCLASS(DefaultToInstanced, EditInlineNew, Abstract)
class BECORE_API UBECharacterMovementCondition : public UObject
{
	GENERATED_BODY()
public:
	/**
	 * CanEnter
	 *
	 *  遷移可能かどうか
	 */
	virtual bool CanEnter(const UBECharacterMovementComponent* CMC) const { return false; }

public:
	//
	// 遷移不可だった時に遷移を推奨する Tag
	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Condition")
	FGameplayTag SuggestStateTag{ FGameplayTag::EmptyTag };
};

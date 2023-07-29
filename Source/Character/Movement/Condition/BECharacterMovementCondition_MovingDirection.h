// Copyright owoDra

#pragma once

#include "Character/Movement/BECharacterMovementCondition.h"

#include "BECharacterMovementCondition_MovingDirection.generated.h"

class UBECharacterMovementComponent;


/**
 * UBECharacterMovementCondition_MovingDirection
 *  
 *  視点角度と進行角度を用いて判定する
 */
UCLASS(meta = (DisplayName = "Condition Moving Direction"))
class UBECharacterMovementCondition_MovingDirection : public UBECharacterMovementCondition
{
	GENERATED_BODY()
public:
	virtual bool CanEnter(const UBECharacterMovementComponent* CMC) const override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Condition")
	float ViewRelativeAngleThreshold = 50.0;
};

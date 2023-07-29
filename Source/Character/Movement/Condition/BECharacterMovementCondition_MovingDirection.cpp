// Copyright owoDra

#include "BECharacterMovementCondition_MovingDirection.h"

#include "Character/Component/BECharacterMovementComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BECharacterMovementCondition_MovingDirection)


bool UBECharacterMovementCondition_MovingDirection::CanEnter(const UBECharacterMovementComponent* CMC) const
{
	if (FMath::Abs(FRotator3f::NormalizeAxis(UE_REAL_TO_FLOAT(
		CMC->GetLocomotionState().InputYawAngle - CMC->GetViewState().Rotation.Yaw))) < ViewRelativeAngleThreshold)
	{
		return true;
	}

	return false;
}

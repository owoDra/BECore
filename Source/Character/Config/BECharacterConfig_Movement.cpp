// Copyright owoDra

#include "BECharacterConfig_Movement.h"

#include "GameplayTag/BETags_Status.h"

#include "Engine/EngineTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BECharacterConfig_Movement)


/////////////////////////////////////////
// FRagdollingConfigs

FRagdollingConfigs::FRagdollingConfigs()
{
	GroundTraceResponses = ECR_Ignore;
}

#if WITH_EDITOR
void FRagdollingConfigs::PostEditChangeProperty(const FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetPropertyName() != GET_MEMBER_NAME_CHECKED(FRagdollingConfigs, GroundTraceObjectTypes))
	{
		return;
	}

	GroundTraceResponses.SetAllChannels(ECR_Ignore);

	for (const auto ObjectType : GroundTraceObjectTypes)
	{
		GroundTraceResponses.SetResponse(UEngineTypes::ConvertToCollisionChannel(ObjectType), ECR_Block);
	}
}
#endif


/////////////////////////////////////////
// FMovementGaitConfigs

FMovementGaitConfigs::FMovementGaitConfigs()
{
	Speeds =
	{
		{ TAG_Status_Gait_Walking	, 175.0 },
		{ TAG_Status_Gait_Running	, 375.0 },
		{ TAG_Status_Gait_Sprinting	, 650.0 }
	};
}

float FMovementGaitConfigs::GetSpeedForGait(const FGameplayTag& Gait) const
{
	if (const float* Speed = Speeds.Find(Gait))
	{
		return *Speed;
	}

	return 0.0f;
}


/////////////////////////////////////////
// FMovementStanceConfigs

FMovementStanceConfigs::FMovementStanceConfigs()
{
	Gaits = 
	{
		{ TAG_Status_Stance_Standing	, FMovementGaitConfigs() },
		{ TAG_Status_Stance_Crouching	, FMovementGaitConfigs() }
	};
}


/////////////////////////////////////////
// FMovementLocomotionModeConfigs

FMovementLocomotionModeConfigs::FMovementLocomotionModeConfigs()
{
	RotationModes =
	{
		{ TAG_Status_RotationMode_VelocityDirection	, FMovementStanceConfigs() },
		{ TAG_Status_RotationMode_ViewDirection		, FMovementStanceConfigs() },
		{ TAG_Status_RotationMode_Aiming			, FMovementStanceConfigs() }
	};
}


/////////////////////////////////////////
// UMovementConfigs

UBECharacterMovementConfigs::UBECharacterMovementConfigs()
{
	Ragdolling.GroundTraceObjectTypes =
	{
		UEngineTypes::ConvertToObjectType(ECC_WorldStatic),
		UEngineTypes::ConvertToObjectType(ECC_WorldDynamic),
		UEngineTypes::ConvertToObjectType(ECC_Destructible)
	};

	Ragdolling.GroundTraceResponses.SetResponse(ECC_WorldStatic, ECR_Block);
	Ragdolling.GroundTraceResponses.SetResponse(ECC_WorldDynamic, ECR_Block);
	Ragdolling.GroundTraceResponses.SetResponse(ECC_Destructible, ECR_Block);

	LocomotionModes =
	{
		{ TAG_Status_LocomotionMode_OnGround, FMovementRotationModeConfigs() },
		{ TAG_Status_LocomotionMode_InAir	, FMovementRotationModeConfigs() },
		{ TAG_Status_LocomotionMode_InWater	, FMovementRotationModeConfigs() }
	};
}

#if WITH_EDITOR
void UBECharacterMovementConfigs::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Ragdolling.PostEditChangeProperty(PropertyChangedEvent);
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif



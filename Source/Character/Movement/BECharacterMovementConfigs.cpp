// Copyright owoDra

#include "BECharacterMovementConfigs.h"

#include "GameplayTag/BETags_Status.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BECharacterMovementConfigs)


/////////////////////////////////////////
// FBECharacterGaitConfigs

FBECharacterGaitConfigs::FBECharacterGaitConfigs(const FGameplayTag& InGaitTag, float InMaxSpeed, float InMaxAcceleration,
	float InBrakingDeceleration, float InGroundFriction, float InJumpZPower, float InAirControl, float InRotationInterpSpeed)
{
	GaitTag = InGaitTag;
	MaxSpeed = InMaxSpeed;
	MaxAcceleration = InMaxAcceleration;
	BrakingDeceleration = InBrakingDeceleration;
	GroundFriction = InGroundFriction;
	JumpZPower = InJumpZPower;
	AirControl = InAirControl;
	RotationInterpSpeed = InRotationInterpSpeed;
}


/////////////////////////////////////////
// FBECharacterStanceConfigs

FBECharacterStanceConfigs::FBECharacterStanceConfigs(const FGameplayTag& InStanceTag)
{
	StanceTag = InStanceTag;

	Gaits =
	{ 
		FBECharacterGaitConfigs(
			TAG_Status_Gait_Walking,
			175.0,
			2500.0,
			2000.0,
			10.0,
			420.0,
			0.25,
			8.0
		),
		FBECharacterGaitConfigs(
			TAG_Status_Gait_Running,
			375.0,
			2500.0,
			2000.0,
			10.0,
			420.0,
			0.5,
			12.0
		)
	};
}


/////////////////////////////////////////
// FBECharacterRotationModeConfigs

FBECharacterRotationModeConfigs::FBECharacterRotationModeConfigs(const FGameplayTag& InRotationModeTag)
{
	RotationModeTag = InRotationModeTag;

	Stances =
	{
		FBECharacterStanceConfigs(TAG_Status_Stance_Standing),
		FBECharacterStanceConfigs(TAG_Status_Stance_Crouching)
	};
}


/////////////////////////////////////////
// FBECharacterLocomotionModeConfigs

FBECharacterLocomotionModeConfigs::FBECharacterLocomotionModeConfigs()
{
	RotationModes =
	{
		FBECharacterRotationModeConfigs(TAG_Status_RotationMode_ViewDirection),
		FBECharacterRotationModeConfigs(TAG_Status_RotationMode_Aiming),
	};
}

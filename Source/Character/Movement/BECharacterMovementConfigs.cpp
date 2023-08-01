// Copyright owoDra

#include "BECharacterMovementConfigs.h"

#include "Character/Movement/BECharacterMovementCondition.h"
#include "GameplayTag/BETags_Status.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BECharacterMovementConfigs)


/////////////////////////////////////////
// FBECharacterGaitConfigs

FBECharacterGaitConfigs::FBECharacterGaitConfigs()
{
	MaxSpeed = 0;
	MaxAcceleration = 0;
	BrakingDeceleration = 0;
	GroundFriction = 1.0;
	JumpZPower = 0;
	AirControl = 0;
	RotationInterpSpeed = 0;
	Condition = nullptr;
}

FBECharacterGaitConfigs::FBECharacterGaitConfigs(float InMaxSpeed, float InMaxAcceleration, float InBrakingDeceleration, float InGroundFriction, float InJumpZPower, float InAirControl, float InRotationInterpSpeed, const TObjectPtr<UBECharacterMovementCondition>& InCondition)
	: MaxSpeed(InMaxSpeed), MaxAcceleration(InMaxAcceleration), BrakingDeceleration(InBrakingDeceleration), GroundFriction(InGroundFriction), JumpZPower(InJumpZPower), AirControl(InAirControl), RotationInterpSpeed(InRotationInterpSpeed), Condition(InCondition)
{

}


/////////////////////////////////////////
// FBECharacterStanceConfigs

FBECharacterStanceConfigs::FBECharacterStanceConfigs()
{
	DefaultGait = TAG_Status_Gait_Walking;

	Gaits =
	{ 
		{
			TAG_Status_Gait_Walking,
			FBECharacterGaitConfigs(
				175.0,
				2500.0,
				2000.0,
				10.0,
				420.0,
				0.25,
				8.0,
				nullptr
			)
		},
		{
			TAG_Status_Gait_Running,
			FBECharacterGaitConfigs(
				375.0,
				2500.0,
				2000.0,
				10.0,
				420.0,
				0.25,
				8.0,
				nullptr
			)
		},
		{
			TAG_Status_Gait_Sprinting,
			FBECharacterGaitConfigs(
				650.0,
				2500.0,
				2000.0,
				10.0,
				420.0,
				0.25,
				8.0,
				nullptr
			)
		},
	};
}

const FBECharacterGaitConfigs& FBECharacterStanceConfigs::GetAllowedGait(const UBECharacterMovementComponent* CMC, const FGameplayTag& DesiredGait, FGameplayTag& OutTag) const
{
	// DesiredGait から GaitConfigs を検索

	const FBECharacterGaitConfigs* Configs{ Gaits.Find(DesiredGait) };
	if (Configs)
	{
		// Condition が設定されていなければそのまま返す

		if (!Configs->Condition)
		{
			OutTag = DesiredGait;
			return *Configs;
		}

		// Condition を検証し遷移可能ならそのまま返す

		if (Configs->Condition->CanEnter(CMC))
		{
			OutTag = DesiredGait;
			return *Configs;
		}

		// 遷移不可なら Condition の SuggestStateTag に基づいて GaitConfigs を返す

		const auto& SuggestTag{ Configs->Condition->SuggestStateTag };

		if (SuggestTag.IsValid())
		{
			return GetAllowedGait(CMC, SuggestTag, OutTag);
		}
	}

	// 遷移可能な GaitConfigs が見つからなかった場合は DefaultGait に基づいて GaitConfigs を返す

	Configs = Gaits.Find(DefaultGait);
	check(Configs);

	OutTag = DefaultGait;
	return *Configs;
}


/////////////////////////////////////////
// FBECharacterRotationModeConfigs

FBECharacterRotationModeConfigs::FBECharacterRotationModeConfigs()
{
	DefaultStance = TAG_Status_Stance_Standing;

	Stances =
	{
		{ TAG_Status_Stance_Standing, FBECharacterStanceConfigs() },
		{ TAG_Status_Stance_Crouching, FBECharacterStanceConfigs() }
	};

	Condition = nullptr;
}

const FBECharacterStanceConfigs& FBECharacterRotationModeConfigs::GetAllowedStance(const FGameplayTag& DesiredStance, FGameplayTag& OutTag) const
{
	// DesiredStance から StanceConfigs を検索

	const FBECharacterStanceConfigs* Configs{ Stances.Find(DesiredStance) };
	if (Configs)
	{
		OutTag = DesiredStance;
		return *Configs;
	}

	// 遷移可能な StanceConfigs が見つからなかった場合は DefaultStance に基づいて StanceConfigs を返す

	Configs = Stances.Find(DefaultStance);
	check(Configs);

	OutTag = DefaultStance;
	return *Configs;
}


/////////////////////////////////////////
// FBECharacterLocomotionModeConfigs

FBECharacterLocomotionModeConfigs::FBECharacterLocomotionModeConfigs()
{
	DefaultRotationMode = TAG_Status_RotationMode_ViewDirection;

	RotationModes =
	{
		{ TAG_Status_RotationMode_VelocityDirection, FBECharacterRotationModeConfigs() },
		{ TAG_Status_RotationMode_ViewDirection, FBECharacterRotationModeConfigs() },
		{ TAG_Status_RotationMode_Aiming, FBECharacterRotationModeConfigs() }
	};
}

const FBECharacterRotationModeConfigs& FBECharacterLocomotionModeConfigs::GetAllowedRotationMode(const UBECharacterMovementComponent* CMC, const FGameplayTag& DesiredRotationMode, FGameplayTag& OutTag) const
{
	// DesiredRotationMode から RotationModeConfigs を検索

	const FBECharacterRotationModeConfigs* Configs{ RotationModes.Find(DesiredRotationMode) };
	if (Configs)
	{
		// Condition が設定されていなければそのまま返す

		if (!Configs->Condition)
		{
			OutTag = DesiredRotationMode;
			return *Configs;
		}

		// Condition を検証し遷移可能ならそのまま返す

		if (Configs->Condition->CanEnter(CMC))
		{
			OutTag = DesiredRotationMode;
			return *Configs;
		}

		// 遷移不可なら Condition の SuggestStateTag に基づいて RotationModeConfigs を返す

		const auto& SuggestTag{ Configs->Condition->SuggestStateTag };

		if (SuggestTag.IsValid())
		{
			return GetAllowedRotationMode(CMC, SuggestTag, OutTag);
		}
	}

	// 遷移可能な RotationModeConfigs が見つからなかった場合は DefaultRotationMode に基づいて RotationModeConfigs を返す

	Configs = RotationModes.Find(DefaultRotationMode);
	check(Configs);

	OutTag = DefaultRotationMode;
	return *Configs;
}

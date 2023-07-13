// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#include "BETargetingSensitivityData.h"

#include "Containers/Set.h"
#include "initializer_list"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BETargetingSensitivityData)


UBETargetingSensitivityData::UBETargetingSensitivityData(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SensitivityMap =
	{
		{ EBEGamepadSensitivity::Slow,			0.5f },
		{ EBEGamepadSensitivity::SlowPlus,		0.75f },
		{ EBEGamepadSensitivity::SlowPlusPlus,	0.9f },
		{ EBEGamepadSensitivity::Normal,		1.0f },
		{ EBEGamepadSensitivity::NormalPlus,	1.1f },
		{ EBEGamepadSensitivity::NormalPlusPlus,1.25f },
		{ EBEGamepadSensitivity::Fast,			1.5f },
		{ EBEGamepadSensitivity::FastPlus,		1.75f },
		{ EBEGamepadSensitivity::FastPlusPlus,	2.0f },
		{ EBEGamepadSensitivity::Insane,		2.5f },
	};
}

const float UBETargetingSensitivityData::SensitivtyEnumToFloat(const EBEGamepadSensitivity InSensitivity) const
{
	if (const float* Sens = SensitivityMap.Find(InSensitivity))
	{
		return *Sens;
	}

	return 1.0f;
}

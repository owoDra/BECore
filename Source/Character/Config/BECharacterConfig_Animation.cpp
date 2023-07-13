// Copyright owoDra

#include "BECharacterConfig_Animation.h"

#include "Engine/EngineTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BECharacterConfig_Animation)


/////////////////////////////////////////
// FAnimFeetConfigs

FAnimFeetConfigs::FAnimFeetConfigs()
{
	IkTraceChannel = TraceTypeQuery1;
}


/////////////////////////////////////////
// FAnimInAirConfigs

FAnimInAirConfigs::FAnimInAirConfigs()
{
	GroundPredictionSweepResponses = ECR_Ignore;
}

#if WITH_EDITOR
void FAnimInAirConfigs::PostEditChangeProperty(const FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetPropertyName() != GET_MEMBER_NAME_CHECKED(FAnimInAirConfigs, GroundPredictionSweepObjectTypes))
	{
		return;
	}

	GroundPredictionSweepResponses.SetAllChannels(ECR_Ignore);

	for (const auto ObjectType : GroundPredictionSweepObjectTypes)
	{
		GroundPredictionSweepResponses.SetResponse(UEngineTypes::ConvertToCollisionChannel(ObjectType), ECR_Block);
	}
}
#endif


/////////////////////////////////////////
// UBECharacterAnimConfigs

UBECharacterAnimConfigs::UBECharacterAnimConfigs()
{
	InAir.GroundPredictionSweepObjectTypes =
	{
		UEngineTypes::ConvertToObjectType(ECC_WorldStatic),
		UEngineTypes::ConvertToObjectType(ECC_WorldDynamic),
		UEngineTypes::ConvertToObjectType(ECC_Destructible)
	};

	InAir.GroundPredictionSweepResponses.SetResponse(ECC_WorldStatic, ECR_Block);
	InAir.GroundPredictionSweepResponses.SetResponse(ECC_WorldDynamic, ECR_Block);
	InAir.GroundPredictionSweepResponses.SetResponse(ECC_Destructible, ECR_Block);
}

void UBECharacterAnimConfigs::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	InAir.PostEditChangeProperty(PropertyChangedEvent);

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

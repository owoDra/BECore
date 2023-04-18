// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BESystemStatics.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/AssetManager.h"
#include "BELogChannels.h"
#include "Components/MeshComponent.h"
#include "GameModes/BEUserFacingExperienceDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BESystemStatics)


TSoftObjectPtr<UObject> UBESystemStatics::GetTypedSoftObjectReferenceFromPrimaryAssetId(FPrimaryAssetId PrimaryAssetId, TSubclassOf<UObject> ExpectedAssetType)
{
	if (UAssetManager* Manager = UAssetManager::GetIfValid())
	{
		FPrimaryAssetTypeInfo Info;
		if (Manager->GetPrimaryAssetTypeInfo(PrimaryAssetId.PrimaryAssetType, Info) && !Info.bHasBlueprintClasses)
		{
			if (UClass* AssetClass = Info.AssetBaseClassLoaded)
			{
				if ((ExpectedAssetType == nullptr) || !AssetClass->IsChildOf(ExpectedAssetType))
				{
					return nullptr;
				}
			}
			else
			{
				UE_LOG(LogBE, Warning, TEXT("GetTypedSoftObjectReferenceFromPrimaryAssetId(%s, %s) - AssetBaseClassLoaded was unset so we couldn't validate it, returning null"),
					*PrimaryAssetId.ToString(),
					*GetPathNameSafe(*ExpectedAssetType));
			}

			return TSoftObjectPtr<UObject>(Manager->GetPrimaryAssetPath(PrimaryAssetId));
		}
	}
	return nullptr;
}

FPrimaryAssetId UBESystemStatics::GetPrimaryAssetIdFromUserFacingExperienceName(const FString& AdvertisedExperienceID)
{
	const FPrimaryAssetType Type(UBEUserFacingExperienceDefinition::StaticClass()->GetFName());
	return FPrimaryAssetId(Type, FName(*AdvertisedExperienceID));
}

void UBESystemStatics::PlayNextGame(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World == nullptr)
	{
		return;
	}

	const FWorldContext& WorldContext = GEngine->GetWorldContextFromWorldChecked(World);
	FURL LastURL = WorldContext.LastURL;

#if WITH_EDITOR
	// To transition during PIE we need to strip the PIE prefix from maps.
	LastURL.Map = UWorld::StripPIEPrefixFromPackageName(LastURL.Map, WorldContext.World()->StreamingLevelsPrefix);
#endif

	FString URL = LastURL.ToString();
	// If we don't remove the host/port info the server travel will fail.
	URL.RemoveFromStart(LastURL.GetHostPortString());

	const bool bAbsolute = false; // we want to use TRAVEL_Relative
	const bool bShouldSkipGameNotify = false;
	World->ServerTravel(URL, bAbsolute, bShouldSkipGameNotify);
}

void UBESystemStatics::SetScalarParameterValueOnAllMeshComponents(AActor* TargetActor, const FName ParameterName, const float ParameterValue, bool bIncludeChildActors)
{
	if (TargetActor != nullptr)
	{
		TargetActor->ForEachComponent<UMeshComponent>(bIncludeChildActors, [=](UMeshComponent* InComponent)
		{
			InComponent->SetScalarParameterValueOnMaterials(ParameterName, ParameterValue);
		});
	}
}

void UBESystemStatics::SetVectorParameterValueOnAllMeshComponents(AActor* TargetActor, const FName ParameterName, const FVector ParameterValue, bool bIncludeChildActors)
{
	if (TargetActor != nullptr)
	{
		TargetActor->ForEachComponent<UMeshComponent>(bIncludeChildActors, [=](UMeshComponent* InComponent)
		{
			InComponent->SetVectorParameterValueOnMaterials(ParameterName, ParameterValue);
		});
	}
}

void UBESystemStatics::SetColorParameterValueOnAllMeshComponents(AActor* TargetActor, const FName ParameterName, const FLinearColor ParameterValue, bool bIncludeChildActors)
{
	SetVectorParameterValueOnAllMeshComponents(TargetActor, ParameterName, FVector(ParameterValue), bIncludeChildActors);
}

TArray<UActorComponent*> UBESystemStatics::FindComponentsByClass(AActor* TargetActor, TSubclassOf<UActorComponent> ComponentClass, bool bIncludeChildActors)
{
	TArray<UActorComponent*> Components;
	if (TargetActor != nullptr)
	{
		TargetActor->GetComponents(ComponentClass, /*out*/ Components, bIncludeChildActors);

	}
	return MoveTemp(Components);
}

///////////////////////////////////////
// RuntimeCurve

float UBESystemStatics::GetRuntimeFloatCurveValue(FRuntimeFloatCurve Curve, float Time)
{
	if (Curve.ExternalCurve)
	{
		return Curve.ExternalCurve->GetFloatValue(Time);
	}

	return Curve.EditorCurveData.Eval(Time);
}

void UBESystemStatics::GetRuntimeFloatCurveValueRange(FRuntimeFloatCurve Curve, float& Min, float& Max)
{
	if (Curve.ExternalCurve)
	{
		Curve.ExternalCurve->GetValueRange(Min, Max);
		return;
	}

	Curve.EditorCurveData.GetValueRange(Min, Max);
}

void UBESystemStatics::GetRuntimeFloatCurveTimeRange(FRuntimeFloatCurve Curve, float& Min, float& Max)
{
	if (Curve.ExternalCurve)
	{
		Curve.ExternalCurve->GetTimeRange(Min, Max);
		return;
	}

	Curve.EditorCurveData.GetTimeRange(Min, Max);
}

FVector UBESystemStatics::GetRuntimeVectorCurveValue(FRuntimeVectorCurve Curve, float Time)
{
	return Curve.GetValue(Time);
}

void UBESystemStatics::GetRuntimeVectorCurveValueRange(FRuntimeVectorCurve Curve, float& Min, float& Max)
{
	if (Curve.ExternalCurve)
	{
		Curve.ExternalCurve->GetValueRange(Min, Max);
		return;
	}

	float Xmin, Xmax;
	Curve.VectorCurves[0].GetValueRange(Xmin, Xmax);

	float Ymin, Ymax;
	Curve.VectorCurves[1].GetValueRange(Ymin, Ymax);

	float Zmin, Zmax;
	Curve.VectorCurves[2].GetValueRange(Zmin, Zmax);

	Min = FMath::Min3(Xmin, Ymin, Zmin);
	Max = FMath::Max3(Xmax, Ymax, Zmax);
}

void UBESystemStatics::GetRuntimeVectorCurveTimeRange(FRuntimeVectorCurve Curve, float& Min, float& Max)
{
	if (Curve.ExternalCurve)
	{
		Curve.ExternalCurve->GetTimeRange(Min, Max);
		return;
	}

	float Xmin, Xmax;
	Curve.VectorCurves[0].GetTimeRange(Xmin, Xmax);

	float Ymin, Ymax;
	Curve.VectorCurves[1].GetTimeRange(Ymin, Ymax);

	float Zmin, Zmax;
	Curve.VectorCurves[2].GetTimeRange(Zmin, Zmax);

	Min = FMath::Min3(Xmin, Ymin, Zmin);
	Max = FMath::Max3(Xmax, Ymax, Zmax);
}
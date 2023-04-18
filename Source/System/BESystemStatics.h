// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "Math/Color.h"
#include "Math/MathFwd.h"
#include "Templates/SubclassOf.h"
#include "UObject/NameTypes.h"
#include "UObject/PrimaryAssetId.h"
#include "UObject/SoftObjectPtr.h"
#include "UObject/UObjectGlobals.h"
#include "Curves/CurveFloat.h"
#include "Curves/CurveVector.h"

#include "BESystemStatics.generated.h"

class AActor;
class UActorComponent;
class UObject;
struct FFrame;
struct FRuntimeVectorCurve;
struct FRuntimeFloatCurve;


UCLASS()
class UBESystemStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/** Returns the soft object reference associated with a Primary Asset Id, this works even if the asset is not loaded */
	UFUNCTION(BlueprintPure, Category = "AssetManager", meta = (DeterminesOutputType = ExpectedAssetType))
		static TSoftObjectPtr<UObject> GetTypedSoftObjectReferenceFromPrimaryAssetId(FPrimaryAssetId PrimaryAssetId, TSubclassOf<UObject> ExpectedAssetType);

	UFUNCTION(BlueprintCallable)
		static FPrimaryAssetId GetPrimaryAssetIdFromUserFacingExperienceName(const FString& AdvertisedExperienceID);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "BE", meta = (WorldContext = "WorldContextObject"))
		static void PlayNextGame(const UObject* WorldContextObject);

	// Sets ParameterName to ParameterValue on all sections of all mesh components found on the TargetActor
	UFUNCTION(BlueprintCallable, Category = "Rendering|Material", meta = (DefaultToSelf = "TargetActor"))
		static void SetScalarParameterValueOnAllMeshComponents(AActor* TargetActor, const FName ParameterName, const float ParameterValue, bool bIncludeChildActors = true);

	// Sets ParameterName to ParameterValue on all sections of all mesh components found on the TargetActor
	UFUNCTION(BlueprintCallable, Category = "Rendering|Material", meta = (DefaultToSelf = "TargetActor"))
		static void SetVectorParameterValueOnAllMeshComponents(AActor* TargetActor, const FName ParameterName, const FVector ParameterValue, bool bIncludeChildActors = true);

	// Sets ParameterName to ParameterValue on all sections of all mesh components found on the TargetActor
	UFUNCTION(BlueprintCallable, Category = "Rendering|Material", meta = (DefaultToSelf = "TargetActor"))
		static void SetColorParameterValueOnAllMeshComponents(AActor* TargetActor, const FName ParameterName, const FLinearColor ParameterValue, bool bIncludeChildActors = true);

	// Gets all the components that inherit from the given class
	UFUNCTION(BlueprintCallable, Category = "Actor", meta = (DefaultToSelf = "TargetActor", ComponentClass = "/Script/Engine.ActorComponent", DeterminesOutputType = "ComponentClass"))
		static TArray<UActorComponent*> FindComponentsByClass(AActor* TargetActor, TSubclassOf<UActorComponent> ComponentClass, bool bIncludeChildActors = true);

	///////////////////////////////////////
	// RuntimeCurve

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Curve")
		static float GetRuntimeFloatCurveValue(FRuntimeFloatCurve Curve, float Time);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Curve")
		static void GetRuntimeFloatCurveValueRange(FRuntimeFloatCurve Curve, float& Min, float& Max);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Curve")
		static void GetRuntimeFloatCurveTimeRange(FRuntimeFloatCurve Curve, float& Min, float& Max);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Curve")
		static FVector GetRuntimeVectorCurveValue(FRuntimeVectorCurve Curve, float Time);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Curve")
		static void GetRuntimeVectorCurveValueRange(FRuntimeVectorCurve Curve, float& Min, float& Max);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Curve")
		static void GetRuntimeVectorCurveTimeRange(FRuntimeVectorCurve Curve, float& Min, float& Max);
};

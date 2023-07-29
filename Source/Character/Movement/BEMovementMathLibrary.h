#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "State/BESpringState.h"

#include "BEMovementMathLibrary.generated.h"

enum class EMovementDirection : uint8;

UCLASS()
class BECORE_API UBEMovementMath : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static constexpr float CounterClockwiseRotationAngleThreshold{ 5.0f };
	static constexpr float TeleportDistanceThresholdSquared{ 2500.0f };

public:
	UFUNCTION(BlueprintPure, Category = "BE|Movement", Meta = (ReturnDisplayName = "Value"))
	static float Clamp01(float Value);

	UFUNCTION(BlueprintPure, Category = "BE|Movement", Meta = (ReturnDisplayName = "Value"))
	static float LerpClamped(float From, float To, float Alpha);

	UFUNCTION(BlueprintPure, Category = "BE|Movement", Meta = (ReturnDisplayName = "Angle"))
	static float LerpAngle(float From, float To, float Alpha);

	UFUNCTION(BlueprintPure, Category = "BE|Movement", Meta = (AutoCreateRefTerm = "From, To", ReturnDisplayName = "Rotator"))
	static FRotator LerpRotator(const FRotator& From, const FRotator& To, float Alpha);

	UFUNCTION(BlueprintPure, Category = "BE|Movement", Meta = (ReturnDisplayName = "Interpolation Ammount"))
	static float ExponentialDecay(float DeltaTime, float Lambda);

	template <typename ValueType>
	static ValueType ExponentialDecay(const ValueType& Current, const ValueType& Target, float DeltaTime, float Lambda);

	UFUNCTION(BlueprintPure, Category = "BE|Movement", Meta = (ReturnDisplayName = "Angle"))
	static float ExponentialDecayAngle(float Current, float Target, float DeltaTime, float Lambda);

	UFUNCTION(BlueprintPure, Category = "BE|Movement", Meta = (ReturnDisplayName = "Angle"))
	static float InterpolateAngleConstant(float Current, float Target, float DeltaTime, float InterpolationSpeed);

	template <typename ValueType, typename StateType>
	static ValueType SpringDamp(const ValueType& Current, const ValueType& Target, StateType& SpringState, float DeltaTime, float Frequency, float DampingRatio, float TargetVelocityAmount = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "BE|Movement", Meta = (ReturnDisplayName = "Value"))
	static float SpringDampFloat(float Current, float Target, UPARAM(ref) FSpringFloatState& SpringState, float DeltaTime, float Frequency, float DampingRatio, float TargetVelocityAmount = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "BE|Movement", Meta = (AutoCreateRefTerm = "Current, Target", ReturnDisplayName = "Vector"))
	static FVector SpringDampVector(const FVector& Current, const FVector& Target, UPARAM(ref) FSpringVectorState& SpringState, float DeltaTime, float Frequency, float DampingRatio, float TargetVelocityAmount = 1.0f);

	UFUNCTION(BlueprintPure, Category = "BE|Movement|Vector", Meta = (AutoCreateRefTerm = "Vector", ReturnDisplayName = "Vector"))
	static FVector ClampMagnitude01(const FVector& Vector);
	static FVector3f ClampMagnitude01(const FVector3f& Vector);

	UFUNCTION(BlueprintPure, Category = "BE|Movement|Vector", Meta = (ReturnDisplayName = "Direction"))
	static FVector RadianToDirectionXY(float Radian);

	UFUNCTION(BlueprintPure, Category = "BE|Movement|Vector", Meta = (ReturnDisplayName = "Direction"))
	static FVector AngleToDirectionXY(float Angle);

	UFUNCTION(BlueprintPure, Category = "BE|Movement|Vector", Meta = (AutoCreateRefTerm = "Direction", ReturnDisplayName = "Angle"))
	static double DirectionToAngle(const FVector2D& Direction);

	UFUNCTION(BlueprintPure, Category = "BE|Movement|Vector", Meta = (AutoCreateRefTerm = "Direction", ReturnDisplayName = "Angle"))
	static double DirectionToAngleXY(const FVector& Direction);

	UFUNCTION(BlueprintPure, Category = "BE|Movement|Vector", Meta = (AutoCreateRefTerm = "Vector", ReturnDisplayName = "Vector"))
	static FVector PerpendicularClockwiseXY(const FVector& Vector);

	UFUNCTION(BlueprintPure, Category = "BE|Movement|Vector", Meta = (AutoCreateRefTerm = "Vector", ReturnDisplayName = "Vector"))
	static FVector PerpendicularCounterClockwiseXY(const FVector& Vector);

	UFUNCTION(BlueprintPure, Category = "BE|Movement|Vector", DisplayName = "Angle Between (Skip Normalization)", Meta = (AutoCreateRefTerm = "From, To", ReturnDisplayName = "Angle"))
	static double AngleBetweenSkipNormalization(const FVector& From, const FVector& To);

	UFUNCTION(BlueprintPure, Category = "BE|Movement|Vector", DisplayName = "Slerp (Skip Normalization)", Meta = (AutoCreateRefTerm = "From, To", ReturnDisplayName = "Direction"))
	static FVector SlerpSkipNormalization(const FVector& From, const FVector& To, float Alpha);

	UFUNCTION(BlueprintCallable, Category = "BE|Movement|Input", Meta = (ReturnDisplayName = "Direction"))
	static EMovementDirection CalculateMovementDirection(float Angle, float ForwardHalfAngle, float AngleThreshold);
};

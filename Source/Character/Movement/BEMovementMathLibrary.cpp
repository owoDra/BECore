#include "BEMovementMathLibrary.h"

#include "Character/Movement/State/BEMovementDirection.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEMovementMathLibrary)

float UBEMovementMath::Clamp01(float Value)
{
	return Value <= 0.0f
		? 0.0f
		: Value >= 1.0f
		? 1.0f
		: Value;
}

float UBEMovementMath::LerpClamped(float From, float To, float Alpha)
{
	return From + (To - From) * Clamp01(Alpha);
}

float UBEMovementMath::LerpAngle(float From, float To, float Alpha)
{
	float Delta = FRotator3f::NormalizeAxis(To - From);

	if (Delta > 180.0f - CounterClockwiseRotationAngleThreshold)
	{
		Delta -= 360.0f;
	}

	return FRotator3f::NormalizeAxis(From + Delta * Alpha);
}

FRotator UBEMovementMath::LerpRotator(const FRotator& From, const FRotator& To, float Alpha)
{
	FRotator Result = (To - From);
	Result.Normalize();

	if (Result.Pitch > 180.0f - CounterClockwiseRotationAngleThreshold)
	{
		Result.Pitch -= 360.0f;
	}

	if (Result.Yaw > 180.0f - CounterClockwiseRotationAngleThreshold)
	{
		Result.Yaw -= 360.0f;
	}

	if (Result.Roll > 180.0f - CounterClockwiseRotationAngleThreshold)
	{
		Result.Roll -= 360.0f;
	}

	Result *= Alpha;
	Result += From;
	Result.Normalize();

	return Result;
}

float UBEMovementMath::ExponentialDecay(float DeltaTime, float Lambda)
{
	return 1.0f - FMath::InvExpApprox(Lambda * DeltaTime);
}

template <typename ValueType>
ValueType UBEMovementMath::ExponentialDecay(const ValueType& Current, const ValueType& Target, const float DeltaTime, const float Lambda)
{
	return Lambda > 0.0f
		? FMath::Lerp(Current, Target, ExponentialDecay(DeltaTime, Lambda))
		: Target;
}

float UBEMovementMath::ExponentialDecayAngle(float Current, float Target, float DeltaTime, float Lambda)
{
	return Lambda > 0.0f
		? LerpAngle(Current, Target, ExponentialDecay(DeltaTime, Lambda))
		: Target;
}

float UBEMovementMath::InterpolateAngleConstant(float Current, float Target, float DeltaTime, float InterpolationSpeed)
{
	if (InterpolationSpeed <= 0.0f || Current == Target)
	{
		return Target;
	}

	auto Delta{ FRotator3f::NormalizeAxis(Target - Current) };

	if (Delta > 180.0f - CounterClockwiseRotationAngleThreshold)
	{
		Delta -= 360.0f;
	}

	const auto Alpha{ InterpolationSpeed * DeltaTime };

	return FRotator3f::NormalizeAxis(Current + FMath::Clamp(Delta, -Alpha, Alpha));
}

template <typename ValueType, typename StateType>
ValueType UBEMovementMath::SpringDamp(const ValueType& Current, const ValueType& Target, StateType& SpringState, const float DeltaTime,
	const float Frequency, const float DampingRatio, const float TargetVelocityAmount)
{
	if (DeltaTime <= UE_SMALL_NUMBER)
	{
		return Current;
	}

	if (!SpringState.bStateValid)
	{
		SpringState.Velocity = ValueType{ 0.0f };
		SpringState.PreviousTarget = Target;
		SpringState.bStateValid = true;

		return Target;
	}

	ValueType Result{ Current };
	FMath::SpringDamper(Result, SpringState.Velocity, Target,
		(Target - SpringState.PreviousTarget) * (Clamp01(TargetVelocityAmount) / DeltaTime),
		DeltaTime, Frequency, DampingRatio);

	SpringState.PreviousTarget = Target;

	return Result;
}

float UBEMovementMath::SpringDampFloat(float Current, float Target, UPARAM(ref)FSpringFloatState& SpringState, float DeltaTime, float Frequency, float DampingRatio, float TargetVelocityAmount)
{
	return SpringDamp(Current, Target, SpringState, DeltaTime, Frequency, DampingRatio, TargetVelocityAmount);
}

FVector UBEMovementMath::SpringDampVector(const FVector& Current, const FVector& Target, UPARAM(ref)FSpringVectorState& SpringState, float DeltaTime, float Frequency, float DampingRatio, float TargetVelocityAmount)
{
	return SpringDamp(Current, Target, SpringState, DeltaTime, Frequency, DampingRatio, TargetVelocityAmount);
}

FVector UBEMovementMath::ClampMagnitude01(const FVector& Vector)
{
	const double MagnitudeSquared = Vector.SizeSquared();

	if (MagnitudeSquared <= 1.0f)
	{
		return Vector;
	}

	const double Scale = FMath::InvSqrt(MagnitudeSquared);

	return FVector(Vector.X * Scale, Vector.Y * Scale, Vector.Z * Scale);
}

FVector3f UBEMovementMath::ClampMagnitude01(const FVector3f& Vector)
{
	const float MagnitudeSquared = Vector.SizeSquared();

	if (MagnitudeSquared <= 1.0f)
	{
		return Vector;
	}

	const float Scale = FMath::InvSqrt(MagnitudeSquared);

	return FVector3f(Vector.X * Scale, Vector.Y * Scale, Vector.Z * Scale);
}

FVector UBEMovementMath::RadianToDirectionXY(float Radian)
{
	float Sin, Cos;
	FMath::SinCos(&Sin, &Cos, Radian);

	return FVector(Cos, Sin, 0.0f);
}

FVector UBEMovementMath::AngleToDirectionXY(float Angle)
{
	return RadianToDirectionXY(FMath::DegreesToRadians(Angle));
}

double UBEMovementMath::DirectionToAngle(const FVector2D& Direction)
{
	return FMath::RadiansToDegrees(FMath::Atan2(Direction.Y, Direction.X));
}

double UBEMovementMath::DirectionToAngleXY(const FVector& Direction)
{
	return FMath::RadiansToDegrees(FMath::Atan2(Direction.Y, Direction.X));
}

FVector UBEMovementMath::PerpendicularClockwiseXY(const FVector& Vector)
{
	return { Vector.Y, -Vector.X, Vector.Z };
}

FVector UBEMovementMath::PerpendicularCounterClockwiseXY(const FVector& Vector)
{
	return { -Vector.Y, Vector.X, Vector.Z };
}

double UBEMovementMath::AngleBetweenSkipNormalization(const FVector& From, const FVector& To)
{
	return FMath::RadiansToDegrees(FMath::Acos(From | To));
}

FVector UBEMovementMath::SlerpSkipNormalization(const FVector& From, const FVector& To, float Alpha)
{
	const auto Dot{ From | To };

	if (Dot > 0.9995f || Dot < -0.9995f)
	{
		return FMath::Lerp(From, To, Alpha).GetSafeNormal();
	}

	const auto Theta{ UE_REAL_TO_FLOAT(FMath::Acos(Dot)) * Alpha };

	float Sin, Cos;
	FMath::SinCos(&Sin, &Cos, Theta);

	const auto FromPerpendicular{ (To - From * Dot).GetSafeNormal() };

	return From * Cos + FromPerpendicular * Sin;
}

EMovementDirection UBEMovementMath::CalculateMovementDirection(float Angle, float ForwardHalfAngle, float AngleThreshold)
{
	if (Angle >= -ForwardHalfAngle - AngleThreshold && Angle <= ForwardHalfAngle + AngleThreshold)
	{
		return EMovementDirection::Forward;
	}

	if (Angle >= ForwardHalfAngle - AngleThreshold && Angle <= 180.0f - ForwardHalfAngle + AngleThreshold)
	{
		return EMovementDirection::Right;
	}

	if (Angle <= -(ForwardHalfAngle - AngleThreshold) && Angle >= -(180.0f - ForwardHalfAngle + AngleThreshold))
	{
		return EMovementDirection::Left;
	}

	return EMovementDirection::Backward;
}

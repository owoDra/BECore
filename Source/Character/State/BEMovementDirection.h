// Copyright owoDra

#pragma once

#include "BEMovementDirection.generated.h"

UENUM(BlueprintType)
enum class EMovementDirection : uint8
{
	Forward,
	Backward,
	Left,
	Right
};

USTRUCT(BlueprintType)
struct BECORE_API FMovementDirectionCache
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (AllowPrivateAccess))
	EMovementDirection MovementDirection{EMovementDirection::Forward};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (AllowPrivateAccess))
	bool bForward{true};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (AllowPrivateAccess))
	bool bBackward{false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (AllowPrivateAccess))
	bool bLeft{false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BE", Meta = (AllowPrivateAccess))
	bool bRight{false};

public:
	constexpr FMovementDirectionCache() = default;

	// ReSharper disable once CppNonExplicitConvertingConstructor
	constexpr FMovementDirectionCache(const EMovementDirection NewMovementDirection)
	{
		MovementDirection = NewMovementDirection;

		bForward = MovementDirection == EMovementDirection::Forward;
		bBackward = MovementDirection == EMovementDirection::Backward;
		bLeft = MovementDirection == EMovementDirection::Left;
		bRight = MovementDirection == EMovementDirection::Right;
	}

	constexpr bool IsForward() const
	{
		return bForward;
	}

	constexpr bool IsBackward() const
	{
		return bBackward;
	}

	constexpr bool IsLeft() const
	{
		return bLeft;
	}

	constexpr bool IsRight() const
	{
		return bRight;
	}

	// ReSharper disable once CppNonExplicitConversionOperator
	constexpr operator EMovementDirection() const
	{
		return MovementDirection;
	}
};

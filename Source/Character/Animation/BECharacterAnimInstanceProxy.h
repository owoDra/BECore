#pragma once

#include "Animation/AnimInstanceProxy.h"

#include "BECharacterAnimInstanceProxy.generated.h"

class UBECharacterAnimInstance;
class UBECharacterLinkedAnimInstance;

/**
 * FBECharacterAnimInstanceProxy
 * 
 *  このクラスは、UBECharacterAnimInstance と UBECharacterLinkedAnimInstance を使用している際に
 *  FAnimInstanceProxy 内の一部の保護されたメンバーへのアクセスするために使用されます
 */
USTRUCT()
struct BECORE_API FBECharacterAnimInstanceProxy : public FAnimInstanceProxy
{
	GENERATED_BODY()

	friend UBECharacterAnimInstance;
	friend UBECharacterLinkedAnimInstance;

public:
	FBECharacterAnimInstanceProxy() = default;

	explicit FBECharacterAnimInstanceProxy(UAnimInstance* AnimationInstance);
};

#include "BECharacterLinkedAnimInstance.h"

#include "BECharacterAnimInstance.h"
#include "BECharacterAnimInstanceProxy.h"

#include "Character/BEPawnMeshAssistInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BECharacterLinkedAnimInstance)


UBECharacterLinkedAnimInstance::UBECharacterLinkedAnimInstance()
{
	RootMotionMode = ERootMotionMode::IgnoreRootMotion;
	bUseMainInstanceMontageEvaluationData = true;
}

void UBECharacterLinkedAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Character = Cast<ABECharacter>(GetOwningActor());
	
#if WITH_EDITOR
	if (!GetWorld()->IsGameWorld())
	{
		// エディターのプレビューにデフォルトのオブジェクトを使用します。

		if (!Parent.IsValid())
		{
			Parent = GetMutableDefault<UBECharacterAnimInstance>();
		}

		if (!IsValid(Character))
		{
			Character = GetMutableDefault<ABECharacter>();
		}
		return;
	}
#endif

	UBEAnimInstance* BEAnimIns{ nullptr };
	IBEPawnMeshAssistInterface::Execute_GetMainAnimInstance(Character, BEAnimIns);

	Parent = Cast<UBECharacterAnimInstance>(BEAnimIns);
}

void UBECharacterLinkedAnimInstance::NativeBeginPlay()
{
	ensureMsgf(Parent.IsValid(), TEXT("Parent is invalid. Parent must inherit from UBECharacterAnimInstance."));

	Super::NativeBeginPlay();
}

FAnimInstanceProxy* UBECharacterLinkedAnimInstance::CreateAnimInstanceProxy()
{
	return new FBECharacterAnimInstanceProxy(this);
}

UBECharacterAnimInstance* UBECharacterLinkedAnimInstance::GetParentUnsafe() const
{
	return Parent.Get();
}

void UBECharacterLinkedAnimInstance::ReinitializeLook()
{
	if (Parent.IsValid())
	{
		Parent->ReinitializeLook();
	}
}

void UBECharacterLinkedAnimInstance::RefreshLook()
{
	if (Parent.IsValid())
	{
		Parent->UpdateLook();
	}
}

void UBECharacterLinkedAnimInstance::SetHipsDirection(const EHipsDirection NewHipsDirection)
{
	if (Parent.IsValid())
	{
		Parent->SetHipsDirection(NewHipsDirection);
	}
}

void UBECharacterLinkedAnimInstance::ActivatePivot()
{
	if (Parent.IsValid())
	{
		Parent->ActivatePivot();
	}
}

void UBECharacterLinkedAnimInstance::ResetJumped()
{
	if (Parent.IsValid())
	{
		Parent->ResetJumped();
	}
}

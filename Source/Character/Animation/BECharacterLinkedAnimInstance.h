#pragma once

#include "Animation/AnimInstance.h"

#include "BECharacterLinkedAnimInstance.generated.h"

class ABECharacter;
class UBECharacterAnimInstance;
enum class EHipsDirection : uint8;

/**
 * UBECharacterLinkedAnimInstance
 * 
 *  Character に適用する Linked Anim Layers 用の
 *  メインのステート処理を行わないサブのアニメーション
 * 
 *  Linked Anim Layers 以外にも FPP 用の AnimInstance としても使用可能
 */
UCLASS()
class BECORE_API UBECharacterLinkedAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	UBECharacterLinkedAnimInstance();

protected:
	//
	// Character のメインの処理を行っている AnimInstance
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Refarence", Transient)
	TWeakObjectPtr<UBECharacterAnimInstance> Parent;

	//
	// この AnimInstance を所有している Character
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Refarence", Transient)
	TObjectPtr<ABECharacter> Character;

public:
	virtual void NativeInitializeAnimation() override;

	virtual void NativeBeginPlay() override;

protected:
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;

protected:
	//
	// PropertyAccess を使用してメインの AnimInstace の変数を読み取る際に使用する
	// 
	// ========== 注意 =========
	// この関数は並列アニメーション評価の前に呼び出されることが保証されているため
	// Parent の UBECharacterAnimationInstance::NativeUpdateAnimation() 
	// の内部でのみ変更される変数を読み取ることは安全です
	// 
	// もし何をしているのかわからないのであれば、Parent 変数を通して変数にアクセスしてください
	//
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BE Linked Animation Instance", Meta = (BlueprintProtected, BlueprintThreadSafe, ReturnDisplayName = "Parent"))
	UBECharacterAnimInstance* GetParentUnsafe() const;

	UFUNCTION(BlueprintCallable, Category = "BE Linked Animation Instance", Meta = (BlueprintProtected, BlueprintThreadSafe))
	void ReinitializeLook();

	UFUNCTION(BlueprintCallable, Category = "BE Linked Animation Instance", Meta = (BlueprintProtected, BlueprintThreadSafe))
	void RefreshLook();

	UFUNCTION(BlueprintCallable, Category = "BE Linked Animation Instance", Meta = (BlueprintProtected, BlueprintThreadSafe))
	void SetHipsDirection(EHipsDirection NewHipsDirection);

	UFUNCTION(BlueprintCallable, Category = "BE Linked Animation Instance", Meta = (BlueprintProtected, BlueprintThreadSafe))
	void ActivatePivot();

	UFUNCTION(BlueprintCallable, Category = "BE Linked Animation Instance", Meta = (BlueprintProtected, BlueprintThreadSafe))
	void ResetJumped();
};

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "BECharacterAnimCurveNameStatics.generated.h"

UCLASS(Meta = (BlueprintThreadSafe))
class BECORE_API UBECharacterAnimCurveNames : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	/////////////////////////////////////
	// Bones

	UFUNCTION(BlueprintPure, Category = "Character Anims|Bones", Meta = (ReturnDisplayName = "Bone Name"))
	static const FName& RootBoneName()
	{
		static const FName Name = FName(TEXTVIEW("root"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Bones", Meta = (ReturnDisplayName = "Bone Name"))
	static const FName& PelvisBoneName()
	{
		static const FName Name = FName(TEXTVIEW("pelvis"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Bones", Meta = (ReturnDisplayName = "Bone Name"))
	static const FName& HeadBoneName()
	{
		static const FName Name = FName(TEXTVIEW("head"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Bones", Meta = (ReturnDisplayName = "Bone Name"))
	static const FName& Spine03BoneName()
	{
		static const FName Name = FName(TEXTVIEW("spine_03"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Bones", Meta = (ReturnDisplayName = "Bone Name"))
	static const FName& FootLeftBoneName()
	{
		static const FName Name = FName(TEXTVIEW("foot_l"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Bones", Meta = (ReturnDisplayName = "Bone Name"))
	static const FName& FootRightBoneName()
	{
		static const FName Name = FName(TEXTVIEW("foot_r"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Bones", Meta = (ReturnDisplayName = "Bone Name"))
	static const FName& HandLeftGunVirtualBoneName()
	{
		static const FName Name = FName(TEXTVIEW("VB hand_l_to_ik_hand_gun"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Bones", Meta = (ReturnDisplayName = "Bone Name"))
	static const FName& HandRightGunVirtualBoneName()
	{
		static const FName Name = FName(TEXTVIEW("VB hand_r_to_ik_hand_gun"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Bones", Meta = (ReturnDisplayName = "Bone Name"))
	static const FName& FootLeftIkBoneName()
	{
		static const FName Name = FName(TEXTVIEW("ik_foot_l"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Bones", Meta = (ReturnDisplayName = "Bone Name"))
	static const FName& FootRightIkBoneName()
	{
		static const FName Name = FName(TEXTVIEW("ik_foot_r"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Bones", Meta = (ReturnDisplayName = "Bone Name"))
	static const FName& FootLeftVirtualBoneName()
	{
		static const FName Name = FName(TEXTVIEW("VB foot_l"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Bones", Meta = (ReturnDisplayName = "Bone Name"))
	static const FName& FootRightVirtualBoneName()
	{
		static const FName Name = FName(TEXTVIEW("VB foot_r"));
		return Name;
	}


	/////////////////////////////////////
	// Animation Slots

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Slots", Meta = (ReturnDisplayName = "Slot Name"))
	static const FName& TransitionSlotName()
	{
		static const FName Name = FName(TEXTVIEW("Transition"));
		return Name;
	}


	/////////////////////////////////////
	// Layering Animation Curves

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerHeadCurveName()
	{
		static const FName Name = FName(TEXTVIEW("LayerHead"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerHeadAdditiveCurveName()
	{
		static const FName Name = FName(TEXTVIEW("LayerHeadAdditive"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerHeadSlotCurveName()
	{
		static const FName Name = FName(TEXTVIEW("LayerHeadSlot"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerArmLeftCurveName()
	{
		static const FName Name = FName(TEXTVIEW("LayerArmLeft"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerArmLeftAdditiveCurveName()
	{
		static const FName Name = FName(TEXTVIEW("LayerArmLeftAdditive"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerArmLeftLocalSpaceCurveName()
	{
		static const FName Name = FName(TEXTVIEW("LayerArmLeftLocalSpace"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerArmLeftSlotCurveName()
	{
		static const FName Name = FName(TEXTVIEW("LayerArmLeftSlot"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerArmRightCurveName()
	{
		static const FName Name = FName(TEXTVIEW("LayerArmRight"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerArmRightAdditiveCurveName()
	{
		static const FName Name = FName(TEXTVIEW("LayerArmRightAdditive"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerArmRightLocalSpaceCurveName()
	{
		static const FName Name = FName(TEXTVIEW("LayerArmRightLocalSpace"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerArmRightSlotCurveName()
	{
		static const FName Name = FName(TEXTVIEW("LayerArmRightSlot"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerHandLeftCurveName()
	{
		static const FName Name = FName(TEXTVIEW("LayerHandLeft"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerHandRightCurveName()
	{
		static const FName Name = FName(TEXTVIEW("LayerHandRight"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerSpineCurveName()
	{
		static const FName Name = FName(TEXTVIEW("LayerSpine"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerSpineAdditiveCurveName()
	{
		static const FName Name = FName(TEXTVIEW("LayerSpineAdditive"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerSpineSlotCurveName()
	{
		static const FName Name = FName(TEXTVIEW("LayerSpineSlot"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerPelvisCurveName()
	{
		static const FName Name = FName(TEXTVIEW("LayerPelvis"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerPelvisSlotCurveName()
	{
		static const FName Name = FName(TEXTVIEW("LayerPelvisSlot"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerLegsCurveName()
	{
		static const FName Name = FName(TEXTVIEW("LayerLegs"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerLegsSlotCurveName()
	{
		static const FName Name = FName(TEXTVIEW("LayerLegsSlot"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& HandLeftIkCurveName()
	{
		static const FName Name = FName(TEXTVIEW("HandLeftIk"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& HandRightIkCurveName()
	{
		static const FName Name = FName(TEXTVIEW("HandRightIk"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& ViewBlockCurveName()
	{
		static const FName Name = FName(TEXTVIEW("ViewBlock"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& AllowAimingCurveName()
	{
		static const FName Name = FName(TEXTVIEW("AllowAiming"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& HipsDirectionLockCurveName()
	{
		static const FName Name = FName(TEXTVIEW("HipsDirectionLock"));
		return Name;
	}


	/////////////////////////////////////
	// Pose Animation Curves

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& PoseGaitCurveName()
	{
		static const FName Name = FName(TEXTVIEW("PoseGait"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& PoseMovingCurveName()
	{
		static const FName Name = FName(TEXTVIEW("PoseMoving"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& PoseStandingCurveName()
	{
		static const FName Name = FName(TEXTVIEW("PoseStanding"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& PoseCrouchingCurveName()
	{
		static const FName Name = FName(TEXTVIEW("PoseCrouching"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& PoseGroundedCurveName()
	{
		static const FName Name = FName(TEXTVIEW("PoseGrounded"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& PoseInAirCurveName()
	{
		static const FName Name = FName(TEXTVIEW("PoseInAir"));
		return Name;
	}


	/////////////////////////////////////
	// Feet Animation Curves

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& FootLeftIkCurveName()
	{
		static const FName Name = FName(TEXTVIEW("FootLeftIk"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& FootLeftLockCurveName()
	{
		static const FName Name = FName(TEXTVIEW("FootLeftLock"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& FootRightIkCurveName()
	{
		static const FName Name = FName(TEXTVIEW("FootRightIk"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& FootRightLockCurveName()
	{
		static const FName Name = FName(TEXTVIEW("FootRightLock"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& FootPlantedCurveName()
	{
		static const FName Name = FName(TEXTVIEW("FootPlanted"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& FeetCrossingCurveName()
	{
		static const FName Name = FName(TEXTVIEW("FeetCrossing"));
		return Name;
	}


	/////////////////////////////////////
	// Other Animation Curves

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& RotationYawSpeedCurveName()
	{
		static const FName Name = FName(TEXTVIEW("RotationYawSpeed"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& RotationYawOffsetCurveName()
	{
		static const FName Name = FName(TEXTVIEW("RotationYawOffset"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& AllowTransitionsCurveName()
	{
		static const FName Name = FName(TEXTVIEW("AllowTransitions"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& SprintBlockCurveName()
	{
		static const FName Name = FName(TEXTVIEW("SprintBlock"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& GroundPredictionBlockCurveName()
	{
		static const FName Name = FName(TEXTVIEW("GroundPredictionBlock"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Character Anims|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& FootstepSoundBlockCurveName()
	{
		static const FName Name = FName(TEXTVIEW("FootstepSoundBlock"));
		return Name;
	}
};
// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BEHUDLayout.h"

#include "BEGameplayTags.h"

#include "CommonUIExtensions.h"
#include "Delegates/Delegate.h"
#include "Input/CommonUIInputTypes.h"
#include "Misc/AssertionMacros.h"
#include "NativeGameplayTags.h"
#include "UITag.h"
#include "UObject/NameTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEHUDLayout)


UBEHUDLayout::UBEHUDLayout(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UBEHUDLayout::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	RegisterUIActionBinding(FBindUIActionArgs(FUIActionTag::ConvertChecked(TAG_UI_Action_Escape), false, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleEscapeAction)));
}

void UBEHUDLayout::HandleEscapeAction()
{
	if (ensure(!EscapeMenuClass.IsNull()))
	{
		UCommonUIExtensions::PushStreamedContentToLayer_ForPlayer(GetOwningLocalPlayer(), TAG_UI_Layer_Menu, EscapeMenuClass);
	}
}
// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "UI/BESimulatedInputWidget.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedPlayerInput.h"
#include "InputAction.h"
#include "BELogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BESimulatedInputWidget)

#define LOCTEXT_NAMESPACE "BESimulatedInputWidget"


UBESimulatedInputWidget::UBESimulatedInputWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetConsumePointerInput(true);
}

#if WITH_EDITOR
const FText UBESimulatedInputWidget::GetPaletteCategory()
{
	return LOCTEXT("PalleteCategory", "Input");
}
#endif // WITH_EDITOR

void UBESimulatedInputWidget::NativeConstruct()
{
	// TODO: We should query the key the simulate whenever the user rebinds a key and after the PlayerController has
	// had it's input initalized. Doing it here will always result in the fallback key being used because
	// the PC does not have any keys mapped in enhanced input.
	QueryKeyToSimulate();
	
	Super::NativeConstruct();
}

FReply UBESimulatedInputWidget::NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	FlushSimulatedInput();
	
	return Super::NativeOnTouchEnded(InGeometry, InGestureEvent);
}

UEnhancedInputLocalPlayerSubsystem* UBESimulatedInputWidget::GetEnhancedInputSubsystem() const
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (ULocalPlayer* LP = GetOwningLocalPlayer())
		{
			return LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		}
	}
	return nullptr;
}

UEnhancedPlayerInput* UBESimulatedInputWidget::GetPlayerInput() const
{
	if (UEnhancedInputLocalPlayerSubsystem* System = GetEnhancedInputSubsystem())
	{
		return System->GetPlayerInput();
	}
	return nullptr;
}

void UBESimulatedInputWidget::InputKeyValue(const FVector& Value)
{
	if (UEnhancedPlayerInput* Input = GetPlayerInput())
	{
		if(KeyToSimulate.IsValid())
		{
			FInputKeyParams Params;
			Params.Delta = Value;
			Params.Key = KeyToSimulate;
			Params.NumSamples = 1;
			Params.DeltaTime = GetWorld()->GetDeltaSeconds();
			Params.bIsGamepadOverride = KeyToSimulate.IsGamepadKey();
			
			Input->InputKey(Params);	
		}
	}
	else
	{
		UE_LOG(LogBE, Error, TEXT("'%s' is attempting to simulate input but has no player input!"), *GetNameSafe(this));
	}
}

void UBESimulatedInputWidget::InputKeyValue2D(const FVector2D& Value)
{
	InputKeyValue(FVector(Value.X, Value.Y, 0.0));
}

void UBESimulatedInputWidget::FlushSimulatedInput()
{
	if (UEnhancedPlayerInput* Input = GetPlayerInput())
	{
		Input->FlushPressedKeys();
	}
}

void UBESimulatedInputWidget::QueryKeyToSimulate()
{
	if (UEnhancedInputLocalPlayerSubsystem* System = GetEnhancedInputSubsystem())
	{
		TArray<FKey> Keys = System->QueryKeysMappedToAction(AssociatedAction);
		if(!Keys.IsEmpty() && Keys[0].IsValid())
		{
			KeyToSimulate = Keys[0];
		}
		else
		{
			KeyToSimulate = FallbackBindingKey;
		}
	}
}

#undef LOCTEXT_NAMESPACE
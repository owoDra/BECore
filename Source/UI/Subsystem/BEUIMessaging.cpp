// Copyright Eigi Chin

#include "BEUIMessaging.h"
#include "BEGameplayTags.h"
#include "Player/BELocalPlayer.h"
#include "PrimaryGameLayout.h"
#include "Messaging/CommonGameDialog.h"


void UBEUIMessaging::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	ConfirmationDialogClassPtr = ConfirmationDialogClass.LoadSynchronous();
	ErrorDialogClassPtr = ErrorDialogClass.LoadSynchronous();
}

void UBEUIMessaging::ShowConfirmation(UCommonGameDialogDescriptor* DialogDescriptor, FCommonMessagingResultDelegate ResultCallback)
{
	if (UBELocalPlayer* LocalPlayer = GetLocalPlayer<UBELocalPlayer>())
	{
		if (UPrimaryGameLayout* RootLayout = LocalPlayer->GetRootUILayout())
		{
			RootLayout->PushWidgetToLayerStack<UCommonGameDialog>(TAG_UI_Layer_Modal, ConfirmationDialogClassPtr, [DialogDescriptor, ResultCallback](UCommonGameDialog& Dialog) {
				Dialog.SetupDialog(DialogDescriptor, ResultCallback);
			});
		}
	}
}

void UBEUIMessaging::ShowError(UCommonGameDialogDescriptor* DialogDescriptor, FCommonMessagingResultDelegate ResultCallback)
{
	if (UBELocalPlayer* LocalPlayer = GetLocalPlayer<UBELocalPlayer>())
	{
		if (UPrimaryGameLayout* RootLayout = LocalPlayer->GetRootUILayout())
		{
			RootLayout->PushWidgetToLayerStack<UCommonGameDialog>(TAG_UI_Layer_Modal, ErrorDialogClassPtr, [DialogDescriptor, ResultCallback](UCommonGameDialog& Dialog) {
				Dialog.SetupDialog(DialogDescriptor, ResultCallback);
			});
		}
	}
}
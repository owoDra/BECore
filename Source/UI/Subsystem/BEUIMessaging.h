// Copyright owoDra

#pragma once

#include "CoreMinimal.h"
#include "Messaging/CommonMessagingSubsystem.h"
#include "Messaging/CommonGameDialog.h"
#include "BEUIMessaging.generated.h"

/**
 * 
 */
UCLASS()
class UBEUIMessaging : public UCommonMessagingSubsystem
{
	GENERATED_BODY()

public:
	UBEUIMessaging() { }

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void ShowConfirmation(UCommonGameDialogDescriptor* DialogDescriptor, FCommonMessagingResultDelegate ResultCallback = FCommonMessagingResultDelegate()) override;
	virtual void ShowError(UCommonGameDialogDescriptor* DialogDescriptor, FCommonMessagingResultDelegate ResultCallback = FCommonMessagingResultDelegate()) override;

private:
	UPROPERTY()
	TSubclassOf<UCommonGameDialog> ConfirmationDialogClassPtr;

	UPROPERTY()
	TSubclassOf<UCommonGameDialog> ErrorDialogClassPtr;

	UPROPERTY(config)
	TSoftClassPtr<UCommonGameDialog> ConfirmationDialogClass;

	UPROPERTY(config)
	TSoftClassPtr<UCommonGameDialog> ErrorDialogClass;
};
// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BEUserFacingExperienceDefinition.h"

#include "CommonSessionSubsystem.h"
#include "Containers/UnrealString.h"
#include "UObject/NameTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEUserFacingExperienceDefinition)


UCommonSession_HostSessionRequest* UBEUserFacingExperienceDefinition::CreateHostingRequest() const
{
	const FString ExperienceName = ExperienceID.PrimaryAssetName.ToString();
	const FString UserFacingExperienceName = GetPrimaryAssetId().PrimaryAssetName.ToString();
	UCommonSession_HostSessionRequest* Result = NewObject<UCommonSession_HostSessionRequest>();
	Result->OnlineMode = ECommonSessionOnlineMode::Online;
	Result->bUseLobbies = true;
	Result->MapID = MapID;
	Result->ModeNameForAdvertisement = UserFacingExperienceName;
	Result->ExtraArgs = ExtraArgs;
	Result->ExtraArgs.Add(TEXT("Experience"), ExperienceName);
	Result->MaxPlayerCount = MaxPlayerCount;

	if (bRecordReplay)
	{
		Result->ExtraArgs.Add(TEXT("DemoRec"), FString());
	}

	return Result;
}

// Copyright Eigi Chin

#pragma once

#include "BEPlayerController.h"

#include "BEPlayerReplayController.generated.h"


/**
 * ABEPlayerReplayController
 *
 *	リプレイのキャプチャと再生に使用される PlayerController
 */
UCLASS()
class ABEPlayerReplayController : public ABEPlayerController
{
	GENERATED_BODY()

	virtual void SetPlayer(UPlayer* InPlayer) override;
};

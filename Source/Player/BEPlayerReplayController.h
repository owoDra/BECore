// Copyright Eigi Chin

#pragma once

#include "BEPlayerController.h"

#include "BEPlayerReplayController.generated.h"


/**
 * ABEPlayerReplayController
 *
 *	���v���C�̃L���v�`���ƍĐ��Ɏg�p����� PlayerController
 */
UCLASS()
class ABEPlayerReplayController : public ABEPlayerController
{
	GENERATED_BODY()

	virtual void SetPlayer(UPlayer* InPlayer) override;
};

// Copyright Eigi Chin

#pragma once

#include "BEPlayerController.h"

#include "BEPlayerReplayController.generated.h"


/**
 * ABEReplayPlayerController
 *
 *	���v���C�̃L���v�`���ƍĐ��Ɏg�p����� PlayerController
 */
UCLASS()
class ABEReplayPlayerController : public ABEPlayerController
{
	GENERATED_BODY()

	virtual void SetPlayer(UPlayer* InPlayer) override;
};

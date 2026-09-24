// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NetworkSyncPlayerController.h"
#include "NetDemoPlayerController.generated.h"

/**
 * Player controller used by the NetworkSync demo.
 */
UCLASS(Blueprintable)
class ANetDemoPlayerController : public ANetworkSyncPlayerController
{
	GENERATED_BODY()

public:
	ANetDemoPlayerController();
};

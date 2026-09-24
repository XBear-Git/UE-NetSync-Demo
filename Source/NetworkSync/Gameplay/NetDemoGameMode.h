// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NetworkSyncGameMode.h"
#include "NetDemoGameMode.generated.h"

/**
 * GameMode used by the NetworkSync demo.
 */
UCLASS(Blueprintable)
class ANetDemoGameMode : public ANetworkSyncGameMode
{
	GENERATED_BODY()

public:
	ANetDemoGameMode();
};

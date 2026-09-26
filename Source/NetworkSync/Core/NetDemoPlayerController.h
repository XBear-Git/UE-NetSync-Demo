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

protected:
	virtual void SetupInputComponent() override;

	/** Cycles the local network emulation preset: Off, 100ms, 150ms, 200ms. */
	void CycleNetworkSimulation();

	/** Applies the selected packet simulation values through UE's built-in Net commands. */
	void ApplyNetworkSimulation(int32 LagMilliseconds, int32 PacketLossPercent);

	/** 0 = Off, 1 = 100ms, 2 = 150ms, 3 = 200ms. */
	int32 NetworkSimulationStep = 0;
};

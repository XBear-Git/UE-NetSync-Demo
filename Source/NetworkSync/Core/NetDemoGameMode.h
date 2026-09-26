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

	/** Starts the server-only delayed respawn flow for a dead character. */
	void ScheduleRespawn(class ANetDemoCharacter* Character, const FVector& DeathLocation);

protected:
	/** Delay between death and server-side respawn. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Respawn")
	float RespawnDelay = 2.0f;

	void RespawnCharacter(class ANetDemoCharacter* Character, FVector DeathLocation);
};

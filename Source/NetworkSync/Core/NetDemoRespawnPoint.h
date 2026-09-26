// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetDemoRespawnPoint.generated.h"

/**
 * Placeable server-side marker used as a respawn destination for NetDemo characters.
 */
UCLASS(Blueprintable, Placeable)
class ANetDemoRespawnPoint : public AActor
{
	GENERATED_BODY()

public:
	ANetDemoRespawnPoint();

	UFUNCTION(BlueprintPure, Category="Respawn")
	FTransform GetRespawnTransform() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Respawn", meta=(AllowPrivateAccess="true"))
	TObjectPtr<class USceneComponent> SceneRoot;
};

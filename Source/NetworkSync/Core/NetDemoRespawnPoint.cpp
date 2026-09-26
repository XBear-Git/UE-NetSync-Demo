// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetDemoRespawnPoint.h"

#include "Components/SceneComponent.h"

ANetDemoRespawnPoint::ANetDemoRespawnPoint()
{
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;
	SetActorEnableCollision(false);
}

FTransform ANetDemoRespawnPoint::GetRespawnTransform() const
{
	return GetActorTransform();
}

// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetDemoGameMode.h"

#include "NetDemoCharacter.h"
#include "NetDemoPlayerController.h"

ANetDemoGameMode::ANetDemoGameMode()
{
	DefaultPawnClass = ANetDemoCharacter::StaticClass();
	PlayerControllerClass = ANetDemoPlayerController::StaticClass();
}

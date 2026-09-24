// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetDemoHealthBar.h"

#include "Components/ProgressBar.h"
#include "GameFramework/Pawn.h"

void UNetDemoHealthBar::NativeConstruct()
{
	Super::NativeConstruct();

	if (!BoundPawn)
	{
		BoundPawn = GetOwningPlayerPawn();
	}

	RefreshOwnerColor();
}

void UNetDemoHealthBar::InitializeForPawn(APawn* InPawn)
{
	BoundPawn = InPawn;
	RefreshOwnerColor();
}

void UNetDemoHealthBar::RefreshOwnerColor()
{
	if (HealthProgressBar && BoundPawn)
	{
		HealthProgressBar->SetFillColorAndOpacity(
			BoundPawn->IsLocallyControlled() ? SelfColor : OtherColor);
	}
}

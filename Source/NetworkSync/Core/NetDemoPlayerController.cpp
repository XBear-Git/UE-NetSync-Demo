// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetDemoPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "InputCoreTypes.h"
#include "../UI/NetDemoSkillMessageWidget.h"

ANetDemoPlayerController::ANetDemoPlayerController()
{
}

void ANetDemoPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController() && SkillMessageWidgetClass)
	{
		SkillMessageWidget = CreateWidget<UNetDemoSkillMessageWidget>(this, SkillMessageWidgetClass);
		if (SkillMessageWidget)
		{
			SkillMessageWidget->AddToViewport(100);
		}
	}
}

void ANetDemoPlayerController::ShowSkillMessage(const FText& Message, float DisplaySeconds)
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	if (!SkillMessageWidget && SkillMessageWidgetClass)
	{
		SkillMessageWidget = CreateWidget<UNetDemoSkillMessageWidget>(this, SkillMessageWidgetClass);
		if (SkillMessageWidget)
		{
			SkillMessageWidget->AddToViewport(100);
		}
	}

	if (SkillMessageWidget)
	{
		SkillMessageWidget->ShowMessage(Message, DisplaySeconds);
	}
}

void ANetDemoPlayerController::ShowSkillMessageIfHidden(const FText& Message)
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	if (!SkillMessageWidget && SkillMessageWidgetClass)
	{
		SkillMessageWidget = CreateWidget<UNetDemoSkillMessageWidget>(this, SkillMessageWidgetClass);
		if (SkillMessageWidget)
		{
			SkillMessageWidget->AddToViewport(100);
		}
	}

	if (SkillMessageWidget && !SkillMessageWidget->IsMessageVisible())
	{
		SkillMessageWidget->ShowMessage(Message);
	}
}

void ANetDemoPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (IsLocalPlayerController() && InputComponent)
	{
		InputComponent->BindKey(EKeys::L, IE_Pressed, this, &ANetDemoPlayerController::CycleNetworkSimulation);
	}
}

void ANetDemoPlayerController::CycleNetworkSimulation()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	NetworkSimulationStep = (NetworkSimulationStep + 1) % 4;

	static const int32 LagOptions[] = { 0, 100, 150, 200 };
	const int32 LagMilliseconds = LagOptions[NetworkSimulationStep];
	const int32 PacketLossPercent = LagMilliseconds > 0 ? 5 : 0;
	ApplyNetworkSimulation(LagMilliseconds, PacketLossPercent);

	const FString Status = LagMilliseconds > 0
		? FString::Printf(TEXT("Network Simulation: ON | Lag: %d ms | Packet Loss: %d%%"), LagMilliseconds, PacketLossPercent)
		: TEXT("Network Simulation: OFF | Lag: 0 ms | Packet Loss: 0%");

	ShowSkillMessage(FText::FromString(Status), 3.0f);
}

void ANetDemoPlayerController::ApplyNetworkSimulation(int32 LagMilliseconds, int32 PacketLossPercent)
{
	const FString OutgoingLagCommand = FString::Printf(TEXT("Net PktLag=%d"), LagMilliseconds);
	const FString IncomingLagCommand = FString::Printf(TEXT("Net PktIncomingLag=%d"), LagMilliseconds);
	const FString OutgoingLossCommand = FString::Printf(TEXT("Net PktLoss=%d"), PacketLossPercent);
	const FString IncomingLossCommand = FString::Printf(TEXT("Net PktIncomingLoss=%d"), PacketLossPercent);

	ConsoleCommand(OutgoingLagCommand);
	ConsoleCommand(IncomingLagCommand);
	ConsoleCommand(OutgoingLossCommand);
	ConsoleCommand(IncomingLossCommand);
}

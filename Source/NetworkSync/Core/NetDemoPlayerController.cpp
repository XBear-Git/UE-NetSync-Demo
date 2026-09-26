// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetDemoPlayerController.h"

#include "Engine/Engine.h"
#include "InputCoreTypes.h"

ANetDemoPlayerController::ANetDemoPlayerController()
{
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

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, Status);
	}
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

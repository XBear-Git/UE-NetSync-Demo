// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NetworkSyncPlayerController.h"
#include "NetDemoPlayerController.generated.h"

class UNetDemoSkillMessageWidget;

/**
 * Player controller used by the NetworkSync demo.
 */
UCLASS(Blueprintable)
class ANetDemoPlayerController : public ANetworkSyncPlayerController
{
	GENERATED_BODY()

public:
	ANetDemoPlayerController();

	/** 只在本地控制器窗口显示技能提示。 */
	void ShowSkillMessage(const FText& Message, float DisplaySeconds = 2.0f);

	/** 仅当本地提示当前隐藏时显示，供技能冷却反馈使用。 */
	void ShowSkillMessageIfHidden(const FText& Message);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	/** Cycles the local network emulation preset: Off, 100ms, 150ms, 200ms. */
	void CycleNetworkSimulation();

	/** Applies the selected packet simulation values through UE's built-in Net commands. */
	void ApplyNetworkSimulation(int32 LagMilliseconds, int32 PacketLossPercent);

	/** 0 = Off, 1 = 100ms, 2 = 150ms, 3 = 200ms. */
	int32 NetworkSimulationStep = 0;

	/** 在 BP_NetDemoPlayerController 中指定技能提示 Widget 蓝图。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI")
	TSubclassOf<UNetDemoSkillMessageWidget> SkillMessageWidgetClass;

	UPROPERTY()
	TObjectPtr<UNetDemoSkillMessageWidget> SkillMessageWidget;
};

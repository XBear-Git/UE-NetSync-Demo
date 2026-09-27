// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NetDemoSkillMessageWidget.generated.h"

class UTextBlock;
class UWidgetAnimation;

/** 本地技能提示 Widget。每个客户端拥有独立实例，不使用全局屏幕消息。 */
UCLASS(Blueprintable)
class UNetDemoSkillMessageWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 在本地窗口显示一条技能提示，并在 DisplaySeconds 后自动隐藏。 */
	UFUNCTION(BlueprintCallable, Category="Skill Message")
	void ShowMessage(const FText& Message, float DisplaySeconds = -1.0f);

	/** 返回提示是否当前正在显示。 */
	UFUNCTION(BlueprintPure, Category="Skill Message")
	bool IsMessageVisible() const;

protected:
	virtual void NativeConstruct() override;

	/** 蓝图中必须存在且命名为 SkillMessageText 的 TextBlock。 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> SkillMessageText;

	/** 默认显示时长，可在 Widget 蓝图 Class Defaults 中调整。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill Message", meta=(ClampMin="0.0"))
	float DisplayDuration = 2.0f;

	/** 蓝图中绑定的提示显示动画；每次 ShowMessage 都会从头播放。 */
	UPROPERTY(Transient, meta=(BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> ShowAnimation;

	FTimerHandle HideTimerHandle;

	void HideMessage();
};

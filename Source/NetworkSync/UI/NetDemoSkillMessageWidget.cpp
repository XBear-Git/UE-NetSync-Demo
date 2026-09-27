// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetDemoSkillMessageWidget.h"

#include "Components/TextBlock.h"

void UNetDemoSkillMessageWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetVisibility(ESlateVisibility::Hidden);
}

void UNetDemoSkillMessageWidget::ShowMessage(const FText& Message, float DisplaySeconds)
{
	if (SkillMessageText)
	{
		SkillMessageText->SetText(Message);
	}

	SetVisibility(ESlateVisibility::HitTestInvisible);

	if (ShowAnimation)
	{
		StopAnimation(ShowAnimation);
		PlayAnimation(ShowAnimation, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.0f);
	}

	GetWorld()->GetTimerManager().ClearTimer(HideTimerHandle);
	const float EffectiveDisplaySeconds = DisplaySeconds >= 0.0f ? DisplaySeconds : DisplayDuration;
	if (EffectiveDisplaySeconds > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(
			HideTimerHandle,
			this,
			&UNetDemoSkillMessageWidget::HideMessage,
			EffectiveDisplaySeconds,
			false);
	}
}

bool UNetDemoSkillMessageWidget::IsMessageVisible() const
{
	return GetVisibility() != ESlateVisibility::Hidden
		&& GetVisibility() != ESlateVisibility::Collapsed;
}

void UNetDemoSkillMessageWidget::HideMessage()
{
	SetVisibility(ESlateVisibility::Hidden);
}

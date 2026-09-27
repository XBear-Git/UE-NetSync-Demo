// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NetDemoHealthBar.generated.h"

class APawn;
class UProgressBar;

/** Blueprint-driven health bar displayed above a NetworkSync demo character. */
UCLASS(Blueprintable)
class UNetDemoHealthBar : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/** Binds this widget to the character whose health it displays. */
	UFUNCTION(BlueprintCallable, Category="Health")
	void InitializeForPawn(APawn* InPawn);

	UFUNCTION(BlueprintImplementableEvent, Category="Health")
	void SetHealthPercent(float Percent);

	UFUNCTION(BlueprintImplementableEvent, Category="Health")
	void SetDeadState(bool bDead);

protected:
	/** Color used when the bound pawn is locally controlled. Set in the Widget Blueprint defaults. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Appearance")
	FLinearColor SelfColor;

	/** Color used when the bound pawn is controlled by another player. Set in the Widget Blueprint defaults. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Appearance")
	FLinearColor OtherColor;

	/** Progress bar whose fill color is updated from SelfColor or OtherColor. */
	UPROPERTY(BlueprintReadOnly, Category="Health", meta=(BindWidgetOptional))
	TObjectPtr<UProgressBar> HealthProgressBar;

	UPROPERTY()
	TObjectPtr<APawn> BoundPawn;

	void RefreshOwnerColor();
};

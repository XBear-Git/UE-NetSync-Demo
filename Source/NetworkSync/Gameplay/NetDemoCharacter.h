// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NetworkSyncCharacter.h"
#include "NetDemoCharacter.generated.h"

/**
 * Networked third-person character used by the NetworkSync demo.
 */
UCLASS(Blueprintable)
class ANetDemoCharacter : public ANetworkSyncCharacter
{
	GENERATED_BODY()

public:
	ANetDemoCharacter();

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** Client request for a server-authoritative projectile spawn. */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRequestFire(const FVector& AimDirection);

protected:
	/** Input action assigned to the fire button in BP_NetDemoCharacter. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<class UInputAction> FireAction;

	/** Projectile class assigned to BP_NetDemoCharacter. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat")
	TSubclassOf<class ANetDemoProjectile> ProjectileClass;

	/** Minimum time between accepted server fire requests. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat")
	float FireCooldown = 0.25f;

	float LastFireTime = -BIG_NUMBER;

	void Fire();
};

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

	virtual void BeginPlay() override;
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintPure, Category="Health")
	float GetCurrentHP() const { return CurrentHP; }

	UFUNCTION(BlueprintPure, Category="Health")
	float GetMaxHP() const { return MaxHP; }

	UFUNCTION(BlueprintPure, Category="Health")
	bool IsDead() const { return bIsDead; }

	/** Client request for a server-authoritative projectile spawn. */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRequestFire(const FVector& AimDirection);

protected:
	/** Death montage configured by BP_NetDemoCharacter. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Death")
	TObjectPtr<class UAnimMontage> DeathMontage;

	/** Prevents the same death montage from being started more than once. */
	bool bDeathAnimationPlayed = false;

	/** Broadcasts the death presentation to all currently connected peers. */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayDeathAnimation();

	void EnterDeathState();
	void PlayDeathAnimation();

	/** World-space widget component used for the overhead health bar. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Health", meta=(AllowPrivateAccess="true"))
	TObjectPtr<class UWidgetComponent> HealthBarComponent;

	/** Runtime instance of the widget assigned to HealthBarComponent. */
	UPROPERTY()
	TObjectPtr<class UNetDemoHealthBar> HealthBarWidget;

	void RefreshHealthBar();

	/** Maximum health configured for this character. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Health")
	float MaxHP = 100.0f;

	/** Server-authoritative health replicated to every client. */
	UPROPERTY(ReplicatedUsing=OnRep_CurrentHP, BlueprintReadOnly, Category="Health")
	float CurrentHP = 100.0f;

	/** Server-authoritative death state, reserved for death and respawn behavior. */
	UPROPERTY(ReplicatedUsing=OnRep_IsDead, BlueprintReadOnly, Category="Health")
	bool bIsDead = false;

	UFUNCTION()
	void OnRep_CurrentHP();

	UFUNCTION()
	void OnRep_IsDead();

	/** Input action assigned to the fire button in BP_NetDemoCharacter. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<class UInputAction> FireAction;

	/** Projectile class assigned to BP_NetDemoCharacter. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat")
	TSubclassOf<class ANetDemoProjectile> ProjectileClass;

	/** Minimum time between accepted server fire requests. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat")
	float FireCooldown = 0.25f;

	/** Socket on the character mesh at the top of the equipped wand. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Fire")
	FName FireSocketName = TEXT("FireSocket");

	/** Small horizontal offset from the wand socket to keep the projectile clear of the wand. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Fire")
	float FireSocketForwardOffset = 20.0f;

	float LastFireTime = -BIG_NUMBER;

	void Fire();
};

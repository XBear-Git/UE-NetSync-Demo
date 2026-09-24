// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetDemoProjectile.generated.h"

class UPointLightComponent;
class UProjectileMovementComponent;
class USphereComponent;
class UStaticMeshComponent;
class UNiagaraComponent;

/**
 * Server-spawned, replicated projectile used by the NetworkSync demo.
 */
UCLASS(Blueprintable)
class ANetDemoProjectile : public AActor
{
	GENERATED_BODY()

public:
	ANetDemoProjectile();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnProjectileHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse,
		const FHitResult& Hit);

	/** Collision root for the projectile. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Projectile", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USphereComponent> CollisionComponent;

	/** Visual mesh configured by BP_NetDemoProjectile. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Projectile", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> ProjectileMesh;

	/** Optional fireball Niagara effect configured by BP_NetDemoProjectile. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Projectile", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UNiagaraComponent> NiagaraEffect;

	/** Optional point light configured by BP_NetDemoProjectile. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Projectile", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UPointLightComponent> PointLight;

	/** Movement component; movement is simulated authoritatively by the server. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Projectile", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	/** Damage applied on a server-side hit. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile")
	float Damage = 25.0f;

	/** Projectile lifetime in seconds. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile")
	float LifeSeconds = 5.0f;
};

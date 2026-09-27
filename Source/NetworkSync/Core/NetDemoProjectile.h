// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetDemoProjectile.generated.h"

class UPointLightComponent;
class UProjectileMovementComponent;
class USphereComponent;
class UStaticMeshComponent;
class UParticleSystemComponent;
class UNiagaraSystem;
class UParticleSystem;
class USoundBase;

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

	UFUNCTION()
	void OnProjectileBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	/** 在所有当前连接的客户端播放命中特效；命中判定仍只由服务器执行。 */
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayHitEffect(FVector_NetQuantize HitLocation, FVector_NetQuantizeNormal HitNormal);

	void HandleProjectileImpact(AActor* OtherActor, const FVector& HitLocation, const FVector& HitNormal);

	/** Collision root for the projectile. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Projectile", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USphereComponent> CollisionComponent;

	/** Visual mesh configured by BP_NetDemoProjectile. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Projectile", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> ProjectileMesh;

	/** Optional Cascade fireball effect configured by BP_NetDemoProjectile. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Projectile", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UParticleSystemComponent> NiagaraEffect;

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

	/** Optional Niagara system played at the server-confirmed hit location. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile|Effects")
	TObjectPtr<UNiagaraSystem> HitEffectSystem;

	/** Optional Cascade particle system played at the server-confirmed hit location. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile|Effects")
	TObjectPtr<UParticleSystem> HitParticleSystem;

	/** Optional sound played at the server-confirmed hit location. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile|Effects")
	TObjectPtr<USoundBase> HitSound;

	/** Prevents Hit and Overlap callbacks from applying damage twice. */
	bool bHasImpacted = false;
};

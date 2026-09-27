// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetDemoProjectile.h"

#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NetDemoCharacter.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"

ANetDemoProjectile::ANetDemoProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(16.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
	// Pawns are handled as overlaps so the projectile cannot physically push them.
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	CollisionComponent->SetGenerateOverlapEvents(true);
	CollisionComponent->SetNotifyRigidBodyCollision(true);
	RootComponent = CollisionComponent;

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(CollisionComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	NiagaraEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("NiagaraEffect"));
	NiagaraEffect->SetupAttachment(CollisionComponent);

	PointLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PointLight"));
	PointLight->SetupAttachment(CollisionComponent);
	PointLight->SetVisibility(false);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = 1800.0f;
	ProjectileMovement->MaxSpeed = 1800.0f;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;

	InitialLifeSpan = LifeSeconds;
}

void ANetDemoProjectile::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(LifeSeconds);

	// Keep the visual mesh from introducing a second collision shape through BP overrides.
	if (ProjectileMesh)
	{
		ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ProjectileMesh->SetGenerateOverlapEvents(false);
	}

	if (IsValid(GetOwner()))
	{
		CollisionComponent->IgnoreActorWhenMoving(GetOwner(), true);
	}

	CollisionComponent->OnComponentHit.AddDynamic(this, &ANetDemoProjectile::OnProjectileHit);
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ANetDemoProjectile::OnProjectileBeginOverlap);
}

void ANetDemoProjectile::OnProjectileHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (!HasAuthority() || !IsValid(OtherActor) || OtherActor == GetOwner())
	{
		return;
	}

	HandleProjectileImpact(OtherActor, Hit.ImpactPoint, Hit.ImpactNormal);
}

void ANetDemoProjectile::OnProjectileBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!HasAuthority() || !IsValid(OtherActor) || OtherActor == GetOwner())
	{
		return;
	}

	const FVector ImpactLocation = bFromSweep
		? FVector(SweepResult.ImpactPoint)
		: GetActorLocation();
	const FVector ImpactNormal = SweepResult.ImpactNormal.IsNearlyZero()
		? FVector(-GetActorForwardVector())
		: FVector(SweepResult.ImpactNormal);
	HandleProjectileImpact(OtherActor, ImpactLocation, ImpactNormal);
}

void ANetDemoProjectile::HandleProjectileImpact(
	AActor* OtherActor,
	const FVector& HitLocation,
	const FVector& HitNormal)
{
	if (bHasImpacted)
	{
		return;
	}

	bHasImpacted = true;
	if (ANetDemoCharacter* OwningCharacter = Cast<ANetDemoCharacter>(GetOwner()))
	{
		// 角色网络通道长期存在，避免短生命周期火球尚未建立客户端通道就被销毁。
		OwningCharacter->MulticastPlayProjectileHitEffect(
			HitLocation,
			HitNormal,
			HitParticleSystem,
			HitEffectSystem,
			HitSound);
	}
	else
	{
		MulticastPlayHitEffect(HitLocation, HitNormal);
	}
	UGameplayStatics::ApplyDamage(OtherActor, Damage, GetInstigatorController(), this, nullptr);
	Destroy();
}

void ANetDemoProjectile::MulticastPlayHitEffect_Implementation(
	FVector_NetQuantize HitLocation,
	FVector_NetQuantizeNormal HitNormal)
{
	if (HitEffectSystem)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			HitEffectSystem,
			HitLocation,
			HitNormal.Rotation());
	}

	if (HitParticleSystem)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			HitParticleSystem,
			HitLocation,
			HitNormal.Rotation());
	}

	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, HitLocation);
	}
}

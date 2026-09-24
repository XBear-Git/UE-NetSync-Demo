// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetDemoCharacter.h"

#include "EnhancedInputComponent.h"
#include "GameFramework/Controller.h"
#include "NetDemoProjectile.h"

ANetDemoCharacter::ANetDemoCharacter()
{
	SetReplicates(true);
	SetReplicateMovement(true);
}

void ANetDemoCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ANetDemoCharacter::Fire);
	}
}

void ANetDemoCharacter::Fire()
{
	if (!IsLocallyControlled() || !GetController())
	{
		return;
	}

	const FVector AimDirection = GetController()->GetControlRotation().Vector();
	ServerRequestFire(AimDirection);
}

bool ANetDemoCharacter::ServerRequestFire_Validate(const FVector& AimDirection)
{
	return FMath::IsFinite(AimDirection.X)
		&& FMath::IsFinite(AimDirection.Y)
		&& FMath::IsFinite(AimDirection.Z)
		&& !AimDirection.IsNearlyZero()
		&& AimDirection.SizeSquared() < 4.0f;
}

void ANetDemoCharacter::ServerRequestFire_Implementation(const FVector& AimDirection)
{
	if (!HasAuthority() || !ProjectileClass || !GetWorld())
	{
		return;
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastFireTime < FireCooldown)
	{
		return;
	}
	LastFireTime = CurrentTime;

	FVector SafeAimDirection = AimDirection.GetSafeNormal();
	if (SafeAimDirection.IsNearlyZero())
	{
		return;
	}

	const FVector SpawnLocation = GetActorLocation() + FVector(0.0f, 0.0f, 60.0f) + SafeAimDirection * 100.0f;
	const FRotator SpawnRotation = SafeAimDirection.Rotation();

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.Instigator = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	GetWorld()->SpawnActor<ANetDemoProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParameters);
}

// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetDemoCharacter.h"

#include "EnhancedInputComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Controller.h"
#include "Net/UnrealNetwork.h"
#include "NetDemoHealthBar.h"
#include "NetDemoProjectile.h"
#include "NetworkSync.h"

ANetDemoCharacter::ANetDemoCharacter()
{
	SetReplicates(true);
	SetReplicateMovement(true);

	HealthBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
	HealthBarComponent->SetupAttachment(RootComponent);
	HealthBarComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 110.0f));
	HealthBarComponent->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarComponent->SetDrawAtDesiredSize(true);
}

void ANetDemoCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		CurrentHP = FMath::Max(0.0f, MaxHP);
		bIsDead = false;
	}

	if (HealthBarComponent)
	{
		HealthBarWidget = Cast<UNetDemoHealthBar>(HealthBarComponent->GetUserWidgetObject());
	}

	if (HealthBarWidget)
	{
		HealthBarWidget->InitializeForPawn(this);
	}

	RefreshHealthBar();
}

void ANetDemoCharacter::RefreshHealthBar()
{
	if (!HealthBarWidget)
	{
		return;
	}

	const float HealthPercent = MaxHP > 0.0f ? CurrentHP / MaxHP : 0.0f;
	HealthBarWidget->SetHealthPercent(FMath::Clamp(HealthPercent, 0.0f, 1.0f));
	HealthBarWidget->SetDeadState(bIsDead);
}

void ANetDemoCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANetDemoCharacter, CurrentHP);
	DOREPLIFETIME(ANetDemoCharacter, bIsDead);
}

float ANetDemoCharacter::TakeDamage(
	float DamageAmount,
	FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser)
{
	if (!HasAuthority() || bIsDead || DamageAmount <= 0.0f)
	{
		return 0.0f;
	}

	const float PreviousHP = CurrentHP;
	CurrentHP = FMath::Clamp(CurrentHP - DamageAmount, 0.0f, MaxHP);
	const float AppliedDamage = PreviousHP - CurrentHP;

	if (CurrentHP <= KINDA_SMALL_NUMBER)
	{
		CurrentHP = 0.0f;
		bIsDead = true;
	}

	RefreshHealthBar();

	UE_LOG(
		LogNetworkSync,
		Log,
		TEXT("[Damage] Target=%s Damage=%.2f HP=%.2f/%.2f Causer=%s"),
		*GetName(),
		AppliedDamage,
		CurrentHP,
		MaxHP,
		*GetNameSafe(DamageCauser));

	return AppliedDamage;
}

void ANetDemoCharacter::OnRep_CurrentHP()
{
	UE_LOG(LogNetworkSync, Verbose, TEXT("[Health] %s received HP %.2f/%.2f"), *GetName(), CurrentHP, MaxHP);
	RefreshHealthBar();
}

void ANetDemoCharacter::OnRep_IsDead()
{
	UE_LOG(LogNetworkSync, Log, TEXT("[Health] %s death state replicated: %s"), *GetName(), bIsDead ? TEXT("Dead") : TEXT("Alive"));
	RefreshHealthBar();
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

	const FRotator ControlRotation = GetController()->GetControlRotation();
	const FVector AimDirection = FRotator(0.0f, ControlRotation.Yaw, 0.0f).Vector();
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

	FVector SafeAimDirection = FVector(AimDirection.X, AimDirection.Y, 0.0f).GetSafeNormal();
	if (SafeAimDirection.IsNearlyZero())
	{
		return;
	}

	FVector SpawnLocation = GetActorLocation() + FVector(0.0f, 0.0f, 60.0f);
	if (GetMesh() && GetMesh()->DoesSocketExist(FireSocketName))
	{
		SpawnLocation = GetMesh()->GetSocketLocation(FireSocketName);
	}
	SpawnLocation += SafeAimDirection * FireSocketForwardOffset;

	const FRotator SpawnRotation = SafeAimDirection.Rotation();

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.Instigator = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	GetWorld()->SpawnActor<ANetDemoProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParameters);
}

// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetDemoCharacter.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "EnhancedInputComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "NetDemoGameMode.h"
#include "../UI/NetDemoHealthBar.h"
#include "NetDemoPlayerController.h"
#include "NetDemoProjectile.h"
#include "NetworkSync.h"
#include "Particles/ParticleSystem.h"

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
		EnterDeathState();

		if (ANetDemoGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ANetDemoGameMode>() : nullptr)
		{
			GameMode->ScheduleRespawn(this, GetActorLocation());
		}
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

	if (bIsDead)
	{
		PlayDeathAnimation();

		if (GetCharacterMovement())
		{
			GetCharacterMovement()->DisableMovement();
		}
	}
	else
	{
		ExitDeathState();
	}
}

void ANetDemoCharacter::EnterDeathState()
{
	if (!HasAuthority())
	{
		return;
	}

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->DisableMovement();
	}

	MulticastPlayDeathAnimation();
}

void ANetDemoCharacter::PlayDeathAnimation()
{
	if (bDeathAnimationPlayed || !DeathMontage || !GetMesh())
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		return;
	}

	bDeathAnimationPlayed = true;
	// 禁止蒙太奇播放结束时自动 Blend Out，让最后一帧持续占据动画槽。
	DeathMontage->bEnableAutoBlendOut = false;
	AnimInstance->Montage_Play(DeathMontage, 1.0f);
}

void ANetDemoCharacter::MulticastPlayDeathAnimation_Implementation()
{
	PlayDeathAnimation();

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->DisableMovement();
	}
}

void ANetDemoCharacter::ExitDeathState()
{
	bDeathAnimationPlayed = false;

	if (GetMesh())
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			if (DeathMontage)
			{
				AnimInstance->Montage_Stop(0.15f, DeathMontage);
			}
		}
	}

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
}

void ANetDemoCharacter::RespawnAtTransform(const FTransform& RespawnTransform)
{
	if (!HasAuthority())
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(RespawnTimerHandle);
	CurrentHP = FMath::Max(0.0f, MaxHP);
	bIsDead = false;

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->StopMovementImmediately();
	}

	SetActorTransform(RespawnTransform, false, nullptr, ETeleportType::TeleportPhysics);
	ExitDeathState();
	ForceNetUpdate();
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
	if (bIsDead || !IsLocallyControlled() || !GetController())
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
	if (!HasAuthority() || bIsDead || !ProjectileClass || !GetWorld())
	{
		return;
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	const float ElapsedSinceLastFire = CurrentTime - LastFireTime;
	const float CooldownSeconds = FMath::Max(0.0f, FireCooldown);
	if (ElapsedSinceLastFire < CooldownSeconds)
	{
		const float RemainingSeconds = FMath::Max(0.0f, CooldownSeconds - ElapsedSinceLastFire);
		ClientNotifySkillCooldown(RemainingSeconds);
		UE_LOG(
			LogNetworkSync,
			Log,
			TEXT("[Combat] Fire request rejected for %s: cooldown %.2f seconds remaining"),
			*GetName(),
			RemainingSeconds);
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

void ANetDemoCharacter::MulticastPlayProjectileHitEffect_Implementation(
	FVector_NetQuantize HitLocation,
	FVector_NetQuantizeNormal HitNormal,
	UParticleSystem* CascadeEffect,
	UNiagaraSystem* NiagaraEffect,
	USoundBase* HitSound)
{
	if (NiagaraEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			NiagaraEffect,
			HitLocation,
			HitNormal.Rotation());
	}

	if (CascadeEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			CascadeEffect,
			HitLocation,
			HitNormal.Rotation());
	}

	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, HitLocation);
	}
}

void ANetDemoCharacter::ClientNotifySkillCooldown_Implementation(float RemainingSeconds)
{
	// Client RPC 理论上只会路由到拥有该 Pawn 的连接；此检查防止错误路由时污染其他窗口。
	if (!IsLocallyControlled())
	{
		return;
	}

	const float SafeRemainingSeconds = FMath::Max(0.0f, RemainingSeconds);
	const FText Message = FText::FromString(FString::Printf(
		TEXT("技能冷却中，还需 %.1f 秒"),
		SafeRemainingSeconds));

	if (ANetDemoPlayerController* OwningPlayerController = Cast<ANetDemoPlayerController>(GetController()))
	{
		OwningPlayerController->ShowSkillMessageIfHidden(Message);
	}

	UE_LOG(
		LogNetworkSync,
		Log,
		TEXT("[Combat] Client received fire cooldown notification: %.2f seconds remaining"),
		SafeRemainingSeconds);
}

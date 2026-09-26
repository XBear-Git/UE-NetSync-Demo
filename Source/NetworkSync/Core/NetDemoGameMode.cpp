// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetDemoGameMode.h"

#include "EngineUtils.h"
#include "NetDemoRespawnPoint.h"
#include "NetDemoCharacter.h"
#include "NetDemoPlayerController.h"
#include "NetworkSync.h"
#include "TimerManager.h"

ANetDemoGameMode::ANetDemoGameMode()
{
	DefaultPawnClass = ANetDemoCharacter::StaticClass();
	PlayerControllerClass = ANetDemoPlayerController::StaticClass();
}

void ANetDemoGameMode::ScheduleRespawn(ANetDemoCharacter* Character, const FVector& DeathLocation)
{
	if (!HasAuthority() || !IsValid(Character) || !GetWorld())
	{
		return;
	}

	FTimerDelegate RespawnDelegate;
	RespawnDelegate.BindUObject(this, &ANetDemoGameMode::RespawnCharacter, Character, DeathLocation);
	GetWorldTimerManager().SetTimer(Character->RespawnTimerHandle, RespawnDelegate, RespawnDelay, false);
}

void ANetDemoGameMode::RespawnCharacter(ANetDemoCharacter* Character, FVector DeathLocation)
{
	if (!HasAuthority() || !IsValid(Character) || !GetWorld() || !Character->IsDead())
	{
		return;
	}

	ANetDemoRespawnPoint* NearestPoint = nullptr;
	float NearestDistanceSquared = TNumericLimits<float>::Max();

	for (TActorIterator<ANetDemoRespawnPoint> It(GetWorld()); It; ++It)
	{
		ANetDemoRespawnPoint* Candidate = *It;
		if (!IsValid(Candidate))
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(DeathLocation, Candidate->GetActorLocation());
		if (DistanceSquared < NearestDistanceSquared)
		{
			NearestDistanceSquared = DistanceSquared;
			NearestPoint = Candidate;
		}
	}

	if (!NearestPoint)
	{
		UE_LOG(LogNetworkSync, Warning, TEXT("No ANetDemoRespawnPoint found for %s; keeping its death location"), *GetNameSafe(Character));
		Character->RespawnAtTransform(FTransform(Character->GetActorRotation(), DeathLocation));
		return;
	}

	Character->RespawnAtTransform(NearestPoint->GetRespawnTransform());
	UE_LOG(
		LogNetworkSync,
		Log,
		TEXT("Respawned %s at %s; nearest point distance %.1f"),
		*GetNameSafe(Character),
		*GetNameSafe(NearestPoint),
		FMath::Sqrt(NearestDistanceSquared));
}

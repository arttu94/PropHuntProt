// Fill out your copyright notice in the Description page of Project Settings.

#include "../Public/GHPropHuntGameMode.h"

#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "EngineGlobals.h"
#include "Engine/Engine.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

#include "Public/GHProp.h"
#include "Public/GHHunter.h"
#include "Public/GHGhost.h"


AGHPropHuntGameMode::AGHPropHuntGameMode()
{
	bDelayedStart = 1;
	RoundTime = 180.0f;
}

void AGHPropHuntGameMode::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	GetWorldTimerManager().SetTimer(TimerHandle_DefaultTimer, this, &AGHPropHuntGameMode::DefaultTimer, GetWorldSettings()->GetEffectiveTimeDilation(), true);
}

void AGHPropHuntGameMode::StartPlay()
{
	Super::StartPlay();
	
	mRoundState = RoundState::Inactive;
}

void AGHPropHuntGameMode::InitGame(const FString & MapName, const FString & Options, FString & ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
}

APawn* AGHPropHuntGameMode::SpawnDefaultPawnFor_Implementation(AController * NewPlayer, AActor * StartSpot)
{
	//Super::SpawnDefaultPawnFor_Implementation(NewPlayer, StartSpot);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (FMath::RandBool())
	{
		if (GhostPlayersToSpawn > 0)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Spawning a ghost"));
			GhostPlayersToSpawn--;
			return GetWorld()->SpawnActor<AGHGhost>(GhostCharacterClass, StartSpot->GetTransform(), SpawnParams);
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Spawning a hunter (else from ghost def)"));
			HunterPlayersToSpawn--;
			return GetWorld()->SpawnActor<AGHHunter>(HunterCharacterClass, StartSpot->GetTransform(), SpawnParams);
		}
	}
	else
	{
		if (HunterPlayersToSpawn > 0)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Spawning a hunter 2"));
			HunterPlayersToSpawn--;
			return GetWorld()->SpawnActor<AGHHunter>(HunterCharacterClass, StartSpot->GetTransform(), SpawnParams);
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Spawning a ghost (else from hunter def)"));
			GhostPlayersToSpawn--;
			return GetWorld()->SpawnActor<AGHGhost>(GhostCharacterClass, StartSpot->GetTransform(), SpawnParams);
		}
	}
}

void AGHPropHuntGameMode::SetMatchState(FName NewState)
{
	Super::SetMatchState(NewState);

}

void AGHPropHuntGameMode::DefaultTimer()
{
	//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, GetMatchState().ToString(), true);

	if (GetMatchState() == MatchState::WaitingToStart)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString("Num players : " + FString::FromInt(GetNumPlayers())), true);
		//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString("Traveling players : " + FString::FromInt(NumTravellingPlayers)), true);
		if (mReadyToStartMatch())
		{
			//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue, TEXT("AGHPropHuntGameMode::InitGame  StartMatchCalled"));
			DividePlayers();
			StartMatch();
			SetRoundState(RoundState::InProgress);
			GetWorldTimerManager().SetTimer(TimerHandle_RoundTimer, this, &AGHPropHuntGameMode::RestartRound, RoundTime, true);
			//RoundStart();
		}
	}
	else if (GetMatchState() == MatchState::InProgress)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, FString("RoundState : " + GetRoundState().ToString()));

		if (GetRoundState() == RoundState::InProgress)
		{
			if (!AnyGhostLeft())
			{
				//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, TEXT("No Ghost alive should reset"));
				if (!GetWorldTimerManager().IsTimerActive(TimerHandle_FinishRoundTimer))
				{
					MatchWinner = Teams::Hunters;
					EndRound();
				}
				//RoundStart();
			}
		}
		else if (GetRoundState() == RoundState::Finished)
		{

		}
	}
}

void AGHPropHuntGameMode::SetRoundState(FName NewState)
{
	mRoundState = NewState;

	if (NewState == RoundState::InProgress)
	{
		//RoundStart();
	}
}

void AGHPropHuntGameMode::RestartRound()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_RoundTimer);
	KillRemainingPlayers();
	DividePlayers();
	RestartPlayers();
	RoundStart();
	GetWorldTimerManager().SetTimer(TimerHandle_RoundTimer, this, &AGHPropHuntGameMode::EndRound, RoundTime, true);
	SetRoundState(RoundState::InProgress);
	MatchWinner = Teams::None;
}

void AGHPropHuntGameMode::EndRound()
{
	DetermineMatchWinner();
	RoundEnd();
	GetWorldTimerManager().SetTimer(TimerHandle_FinishRoundTimer, this, &AGHPropHuntGameMode::RestartRound, 3.0f, false);
	SetRoundState(RoundState::Finished);
}

void AGHPropHuntGameMode::DetermineMatchWinner()
{
	if (MatchWinner == Teams::None)
	{
		MatchWinner = Teams::Ghosts;
	}
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, MatchWinner.ToString(), true);
}

float AGHPropHuntGameMode::GetRemainingRoundTime() const
{
	return GetWorldTimerManager().GetTimerRemaining(TimerHandle_RoundTimer);
}

int AGHPropHuntGameMode::GetRemainingGhostPlayerCount()
{
	int CurrentCount = 0;

	for (TActorIterator<AGHGhost> It(GetWorld()); It; ++It)
	{
		CurrentCount++;
	}

	RemainingGhostPlayerCount = CurrentCount;

	//if(RemainingGhostPlayerCount)
	return RemainingGhostPlayerCount;
}

bool AGHPropHuntGameMode::mReadyToStartMatch()
{
	//Super::ReadyToStartMatch();

	if (GetNumPlayers() >= 2)
	{
		if (NumTravellingPlayers == 0)
		{
			//DividePlayers();

			return true;
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue, FString("ready to start match"));
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

void AGHPropHuntGameMode::DividePlayers()
{
	//GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, TEXT("AGHPropHuntGameMode::DividePlayers() executed"));

	GhostPlayersToSpawn = 0;
	HunterPlayersToSpawn = 0;

	GhostPlayersToSpawn = GetNumPlayers() / 2;
	HunterPlayersToSpawn = GetNumPlayers() - GhostPlayersToSpawn;
	
	RemainingGhostPlayerCount = GhostPlayersToSpawn;

	//GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Green, FString("Ghost Players To Spawn : " + FString::FromInt(GhostPlayersToSpawn)));
	//GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Green, FString("Hunter Players To Spawn : " + FString::FromInt(HunterPlayersToSpawn)));
}

bool AGHPropHuntGameMode::AnyGhostLeft()
{
	bool AnyGhostLeft = false;
	for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
	{
		if (It->Get()->GetPawn() != nullptr)
		{
			if (It->Get()->GetPawn()->GetClass()->IsChildOf(AGHGhost::StaticClass()) || It->Get()->GetPawn()->GetClass()->IsChildOf(AGHProp::StaticClass()))
			{
				AnyGhostLeft = true;
				break;
			}
		}
	}

	return AnyGhostLeft;
}

void AGHPropHuntGameMode::KillRemainingPlayers()
{
	//GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Green, FString("KillRemainingPlayers"));

	//for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
	//{
	//	//APlayerController* Controller = Cast<APlayerController>(It->Get());
	//	if (It->Get()->GetPawn() != nullptr)
	//	{
	//		//It->Get()->UnPossess();
	//		if (It->Get()->GetPawn()->GetClass()->IsChildOf(AGHHunter::StaticClass()))
	//		{
	//			Cast<AGHHunter>(It->Get()->GetPawn())->DestroyWithWeapons();
	//		}
	//		else if(It->Get()->GetPawn()->GetClass()->IsChildOf(AGHProp::StaticClass()))
	//		{
	//			It->Get()->GetPawn()->Destroy();
	//		}
	//	}
	//}

	for (TActorIterator<AGHHunter> It(GetWorld()); It; ++It)
	{
		It->DestroyWithWeapons();
	}
	for (TActorIterator<AGHGhost> It(GetWorld()); It; ++It)
	{
		It->Destroy();
	}
	for (TActorIterator<AGHProp> It(GetWorld()); It; ++It)
	{
		if (It->IsControlled())
		{
			It->GetController()->UnPossess();
		}
		It->ResetToOriginalLocationAndRotator();
	}
}

void AGHPropHuntGameMode::RestartPlayers()
{
	//GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Green, FString("RestartingPlayers"));

	for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
	{
		//APlayerController* Controller = Cast<APlayerController>(It->Get());
		RestartPlayer(It->Get());
	}
}

FTransform AGHPropHuntGameMode::GetRandomPlayerStart() const
{
	TArray<APlayerStart*> PlayerSpawns;

	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		APlayerStart* tmp = *It;
		PlayerSpawns.Add(tmp);
	}
	return PlayerSpawns[FMath::RandRange(0, PlayerSpawns.Num() - 1)]->GetActorTransform();
}

void AGHPropHuntGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GHPropHuntGameMode.generated.h"

/** Possible state of the current match, where a match is all the gameplay that happens on a single map */
namespace RoundState
{
	const FName Preparation;
	const FName InProgress;
	const FName Finished;
	const FName Inactive;
}

namespace Teams
{
	const FName None = "None";
	const FName Hunters = "Hunters";
	const FName Ghosts = "Ghosts";
}

/**
 * 
 */
UCLASS()
class GHOSTHUNT_API AGHPropHuntGameMode : public AGameMode
{
	GENERATED_BODY()

public:

	AGHPropHuntGameMode();

protected: 

	virtual void StartPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Classes)
	TSubclassOf<ACharacter> HunterCharacterClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Classes)
	TSubclassOf<ACharacter> GhostCharacterClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Classes)
	TSubclassOf<APawn> PropBaseClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerData)
	TArray<APlayerController*> PlayerControllers;
	//TArray<TSubclassOf<APlayerController>> PlayerControllers;

	UFUNCTION(BlueprintCallable, Category = PlayerData)
	void DividePlayers();

	bool AnyGhostLeft();

	void KillRemainingPlayers();
	void RestartPlayers();

	void DefaultTimer();

	FName GetRoundState() const { return mRoundState; }

	void SetRoundState(FName NewState);

	void RestartRound();
	void EndRound();

	UFUNCTION(BlueprintImplementableEvent)
	void RoundStart();

	UFUNCTION(BlueprintImplementableEvent)
	void RoundEnd();

	void DetermineMatchWinner();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RoundInfo)
	FName MatchWinner;

	UFUNCTION(BlueprintCallable)
	float GetRemainingRoundTime() const;

	UFUNCTION(BlueprintCallable)
    int GetRemainingGhostPlayerCount();

	int RemainingGhostPlayerCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerData)
	int GhostPlayersToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerData)
	int	HunterPlayersToSpawn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = RoundData)
	float RoundTime;

	FTimerHandle TimerHandle_DefaultTimer;

	FTimerHandle TimerHandle_RoundTimer;

	FTimerHandle TimerHandle_FinishRoundTimer;

	FName mRoundState;

public:

	bool mReadyToStartMatch();

	virtual void Tick(float DeltaSeconds) override;

	virtual void SetMatchState(FName NewState) override;

	virtual void PreInitializeComponents() override;

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot) override;

	FTransform GetRandomPlayerStart() const;

};

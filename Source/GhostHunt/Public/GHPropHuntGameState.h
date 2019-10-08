// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "GHPropHuntGameState.generated.h"

/**
 * 
 */
UCLASS()
class GHOSTHUNT_API AGHPropHuntGameState : public AGameState
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Classes)
	TSubclassOf<APawn> PropBaseClass;

public:

	AGHPropHuntGameState();

	virtual void BeginPlay() override;

	//UPROPERTY(Transient, Replicated, BlueprintReadOnly)
	int RemainingGhostPlayers;
	
	void SpawnAllProps();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastDestroyOriginalMeshActor(AActor* actorToDestroy);

};

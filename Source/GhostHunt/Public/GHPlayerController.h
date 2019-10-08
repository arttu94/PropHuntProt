// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GHPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class GHOSTHUNT_API AGHPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:


public: 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UserSettings)
	float MouseSens;
	
};

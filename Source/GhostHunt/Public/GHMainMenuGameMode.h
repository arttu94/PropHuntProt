// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GHMainMenuGameMode.generated.h"

/**
 * 
 */
UCLASS()
class GHOSTHUNT_API AGHMainMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, Category = "Utility")
	TArray<FString> GetAllMapNames();

	UFUNCTION(BlueprintCallable, Category = "Utility")
	bool OpenLevelPak();

	UFUNCTION(BlueprintCallable, Category = "Utility")
	void MountPak(FString PakPath);
	
};

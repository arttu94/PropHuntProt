// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GHPropMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class GHOSTHUNT_API UGHPropMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()
	
protected:
	float MovingSpeed;

public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	void SetMovingSpeed(float Speed);

};

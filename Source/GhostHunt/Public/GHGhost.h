// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GHGhost.generated.h"

class UCameraComponent;
class AActor;
class APawn;
class UMaterialParameterCollection;
class APHPropPawn;
class USpringArmComponent;

UCLASS()
class GHOSTHUNT_API AGHGhost : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AGHGhost();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Components)
	USpringArmComponent* SpringArmComp;
	UCameraComponent* CameraComp;

	FVector2D MovementInput;
	FVector2D CameraInput;

	float InitialHealth;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	float Health;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	float Pitch;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float AxisValue);
	void MoveRight(float AxisValue);
	void PitchCamera(float AxisValue);
	void YawCamera(float AxisValue);

	void SetPitch();

	UFUNCTION(reliable, server, WithValidation)
	void ServerSetPitch();

	//void SetCollisionEnabled(bool enabled);

	//UFUNCTION(reliable, server, WithValidation)
	//void ServerSetCollisionEnabled(bool enabled);

	void PossessObject();

	UFUNCTION(reliable, server, WithValidation)
	void ServerPossessObject();

	UFUNCTION(reliable, server, WithValidation)
	void ServerUnPossessObject();

	UFUNCTION(BlueprintImplementableEvent)
	void Die();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GrassStuff)
	UMaterialParameterCollection* MPC_Grass;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Possessing)
	APawn* PossessedPawn;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual float TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	float GetHealth() const;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void UnPossessObject();

	UFUNCTION(NetMulticast, reliable)
	void MulticastSetActorEnabledCollision(bool bCol);
};

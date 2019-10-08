// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GHProp.generated.h"

class UGHPropMovementComponent;
class UCameraComponent;
class USoundBase;
class USoundAttenuation;
class USpringArmComponent;
class UStaticMeshComponent;

UCLASS()
class GHOSTHUNT_API AGHProp : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AGHProp();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Components)
	USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Components)
	UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Components)
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Components)
	UGHPropMovementComponent* PawnMovementComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Movement)
	float MovingSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Movement)
	float JumpForce;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Movement)
	bool bCanStepOnGrass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Movement)
	float CameraDistScrollMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GrassStuff)
	UMaterialParameterCollection* MPC_Grass;

	FVector2D MovementInput;
	FVector2D CameraInput;

	bool bGrounded;

	FVector OriginalLocation;
	FRotator OriginalRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
	TArray<USoundBase*> TauntSFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
	USoundAttenuation * SoundAttenuationSettings;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual UPawnMovementComponent* GetMovementComponent() const override;

	void Move(FVector Direction);

	UFUNCTION(reliable, server, WithValidation)
	void ServerMove(FVector Direction);

	void MoveForward(float AxisValue);
	void MoveRight(float AxisValue);
	void PitchCamera(float AxisValue);
	void YawCamera(float AxisValue);

	void Jump();

	UFUNCTION(reliable, server, WithValidation)
	void ServerJump();

	void ZoomCamera(float AxisValue);

	void UnPossessPawn();

	UFUNCTION(reliable, server, WithValidation)
	void ServerUnPossessPawn();

	UFUNCTION(BlueprintImplementableEvent)
	void PossessingActorDied();

	void PlayRandomSFX();

	UFUNCTION(Server, unreliable, WithValidation)
	void ServerPlayRandomSFX();

	UFUNCTION(NetMulticast, unreliable)
	void MulticastPlayRandomSFX(int index);

	void SetVectorParameter();

	UFUNCTION(Server, unreliable, WithValidation)
	void ServerSetVectorParameter();

	UFUNCTION(NetMulticast, unreliable)
	void MulticastSetVectorParameter();

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

public:

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = Possessing)
	APawn* PossessingActor;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual float TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	void SetPossessingActor(APawn* Actor);

	//UFUNCTION(Server, reliable, WithValidation)
	//void ServerSetPossessingActor(APawn* Actor);

	void ResetToOriginalLocationAndRotator();
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GHHunter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class AGHWeapon;
class UStaticMeshComponent;

UCLASS()
class GHOSTHUNT_API AGHHunter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AGHHunter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, category = Components)
	UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, category = Components)
	USpringArmComponent* SpringArmComp;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite)
	AGHWeapon* ActiveWeapon;

	UPROPERTY(EditDefaultsOnly, Category = Player)
	TSubclassOf<AGHWeapon> PrimaryWeaponClass;

	UPROPERTY(VisibleDefaultsOnly, Category = Player)
	FName WeaponAttachSocketName;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, category = Cosmetics)
	USkeletalMeshComponent* MaskComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, category = Cosmetics)
	USkeletalMeshComponent* HatComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GrassStuff)
	UMaterialParameterCollection* MPC_Grass;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	float Pitch;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	bool bCrouching;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	bool bJumping;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	bool bFiring;

	void StartJump();
	void StopJump();
	void StartCrouch();
	void StopCrouch();

	void Moveforward(float AxisValue);
	void MoveRight(float AxisValue);
	void PitchCamera(float AxisValue);
	void YawCamera(float AxisValue);

	void StartFire();
	void StopFire();

public:	

	void SetPitch();

	UFUNCTION(reliable, server, WithValidation)
	void ServerSetPitch();

	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual FVector GetPawnViewLocation() const override;

	void DestroyWithWeapons();
	
};

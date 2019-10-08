// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GHWeapon.generated.h"

class UDamageType;
class USoundBase;
class UParticleSystem;
class UStaticMeshComponent;
class USoundAttenuation;

USTRUCT()
struct FHitScanTrace
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TEnumAsByte<EPhysicalSurface> SurfaceType;

	UPROPERTY()
	FVector_NetQuantize TraceTo;

	UPROPERTY()
	int ForceNetUpdate;
};

UCLASS()
class GHOSTHUNT_API AGHWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGHWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	UStaticMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	TSubclassOf<UDamageType> DamageType;

	//UPROPERTY(EditDefaultsOnly, Category = Weapon)
	//TSubclassOf<UCameraShake> FireCamShake;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	FName MuzzleSocketName;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	FName TracerTargetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	UParticleSystem* MuzzleEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	USoundBase* SoundEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	USoundBase* NoAmmoSoundEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	USoundAttenuation* AttenuationEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	UParticleSystem* DefaultImpactEffect;

	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	//UParticleSystem* TracerEffect;

	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	float BaseDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Ammo)
	int BaseAmmo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Ammo)
	int CurrentAmmo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Ammo)
	float ImpactImpulseForce;

	virtual void Fire();

	void SFX();

	UFUNCTION(NetMulticast, unreliable)
	void MulticastSFX();

	UFUNCTION(server, reliable, WithValidation)
	void ServerSFX();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire();

	FTimerHandle TimerHandle_TimeBetweenShots;

	float LastFireTime;

	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	float RateOfFire;

	float TimeBetweenShots;

	UPROPERTY(ReplicatedUsing = OnRep_HitScanTrace)
	FHitScanTrace HitScanTrace;

	UFUNCTION()
	void OnRep_HitScanTrace();

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Weapon)
	bool bHasRecoil;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Weapon)
	float VerticalRecoilForce;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Weapon)
	FVector2D HorizontalRecoilForceRange;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void StartFire();

	virtual void StopFire();

	void PlayFireEffects(const FVector &TracerEndPoint);

	void PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint);

	void PlaySoundEffects();
	
};

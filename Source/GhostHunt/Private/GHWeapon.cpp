// Fill out your copyright notice in the Description page of Project Settings.

#include "../Public/GHWeapon.h"
#include "../Public/GHProp.h"
#include "../Public/GHGhost.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include <PhysicalMaterials/PhysicalMaterial.h>
#include "Public/GHPlayerController.h"
#include "Public/GHHunter.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "GhostHunt.h"

// Sets default values
AGHWeapon::AGHWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "muzzle_socket";
	TracerTargetName = "BeamEnd";

	BaseDamage = 20.f;
	RateOfFire = 50.0f;

	SetReplicates(true);

	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;

	HitScanTrace.ForceNetUpdate = 0;
}

// Called when the game starts or when spawned
void AGHWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	TimeBetweenShots = 60 / RateOfFire;

	CurrentAmmo = BaseAmmo;
}

// Called every frame
void AGHWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGHWeapon::Fire()
{
	if (Role < ROLE_Authority)
	{
		ServerFire();

		ServerSFX();
	}
	else
	{
		MulticastSFX();
	}

	if (CurrentAmmo > 0)
	{
		AActor* mOwner = GetOwner();
		if (mOwner)
		{
			FRotator EyeRot;
			FVector EyeLoc;
			mOwner->GetActorEyesViewPoint(EyeLoc, EyeRot);

			FVector ShotDirection = EyeRot.Vector();
			FVector TraceEnd = EyeLoc + (ShotDirection * 10000);

			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(mOwner);
			QueryParams.AddIgnoredActor(this);
			QueryParams.bTraceComplex = true;
			QueryParams.bReturnPhysicalMaterial = true;

			FVector TracerEndPoint = TraceEnd;
			EPhysicalSurface SurfaceType = SurfaceType_Default;

			if (bHasRecoil)
			{
				AGHHunter* hunter = Cast<AGHHunter>(mOwner);
				if (hunter && hunter->GetController())
				{
					hunter->AddControllerPitchInput(VerticalRecoilForce);
					hunter->AddControllerYawInput(FMath::RandRange(HorizontalRecoilForceRange.X, HorizontalRecoilForceRange.Y));

					if (Role == ROLE_Authority)
						hunter->SetPitch();
					else
						hunter->ServerSetPitch();
				}
			}

			FHitResult Hit;
			if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLoc, TraceEnd, COLLISION_WEAPON, QueryParams))
			{
				AActor* HitActor = Hit.GetActor();

				SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

				float ActualDamage = BaseDamage;

				AGHProp* Prop = Cast<AGHProp>(HitActor);

				if (Prop)
				{
					UStaticMeshComponent* SM = Cast<UStaticMeshComponent>(Prop->GetComponentByClass(UStaticMeshComponent::StaticClass()));
					if (SM)
					{
						SM->AddImpulse(ShotDirection * ImpactImpulseForce * SM->GetMass());
					}
				}

				AGHGhost* Ghost = Cast<AGHGhost>(HitActor);

				TSubclassOf<UDamageType> const ValidDamageTypeClass = TSubclassOf<UDamageType>(UDamageType::StaticClass());
				FDamageEvent DamageEvent(ValidDamageTypeClass);

				const float DamageAmount = 60.0f;

				//APlayerController* PlayerController = Cast<APlayerController>(GetParentActor()->GetInstigatorController());
				if (HitActor)
				{
					HitActor->TakeDamage(ActualDamage, DamageEvent, nullptr, this);
				}


				PlayImpactEffects(SurfaceType, Hit.ImpactPoint);

				TracerEndPoint = Hit.ImpactPoint;
			}

			if (Role == ROLE_Authority)
			{
				HitScanTrace.TraceTo = TracerEndPoint;
				HitScanTrace.SurfaceType = SurfaceType;
				//HitScanTrace.ForceNetUpdate = HitScanTrace.ForceNetUpdate >= 10 ? 0 : HitScanTrace.ForceNetUpdate+1;
				HitScanTrace.ForceNetUpdate++;
				//if (HitScanTrace.ForceNetUpdate >= 100)
				//{
				//	HitScanTrace.ForceNetUpdate = 0;
				//}
			}

			//PlaySoundEffects();

			PlayFireEffects(TracerEndPoint);

			LastFireTime = GetWorld()->TimeSeconds;
			CurrentAmmo--;
		}
	}
}

void AGHWeapon::OnRep_HitScanTrace()
{
	PlayFireEffects(HitScanTrace.TraceTo);

	PlayImpactEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);

	//HitScanTrace.ForceNetUpdate = HitScanTrace.ForceNetUpdate >= 10 ? 0 : HitScanTrace.ForceNetUpdate + 1;
}

void AGHWeapon::SFX()
{
	PlaySoundEffects();
}

void AGHWeapon::MulticastSFX_Implementation()
{
	SFX();
}

void AGHWeapon::ServerSFX_Implementation()
{
	MulticastSFX();
}

bool AGHWeapon::ServerSFX_Validate()
{
	return true;
}

void AGHWeapon::ServerFire_Implementation()
{
	Fire();
}

bool AGHWeapon::ServerFire_Validate()
{
	return true;
}

void AGHWeapon::StartFire()
{
	float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);

	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &AGHWeapon::Fire, TimeBetweenShots, true, FirstDelay);
}

void AGHWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

void AGHWeapon::PlayFireEffects(const FVector &TracerEndPoint)
{
	if (MuzzleEffect)
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);


	//if (TracerEffect)
	//{
	//	FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
	//	UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
	//	if (TracerComp)
	//	{
	//		TracerComp->SetVectorParameter(TracerTargetName, TracerEndPoint);
	//	}
	//}

	//APawn* mOwner = Cast<APawn>(GetOwner());
	//if (mOwner)
	//{
	//	APlayerController* PC = Cast<APlayerController>(mOwner->GetController());
	//	if (PC)
	//	{
	//		PC->ClientPlayCameraShake(FireCamShake);
	//	}
	//}
}

void AGHWeapon::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
	UParticleSystem* SelectedEffect = nullptr;

	SelectedEffect = DefaultImpactEffect;

	if (SelectedEffect)
	{
		FVector ShotDirection = ImpactPoint - MeshComp->GetSocketLocation(MuzzleSocketName);
		ShotDirection.Normalize();

		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ShotDirection.Rotation());
	}
}

void AGHWeapon::PlaySoundEffects()
{
	if (CurrentAmmo > 0)
	{
		if (SoundEffect)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundEffect, GetActorLocation(), 0.5f, 1.f, 0.f, AttenuationEffect);
		}
	}
	else
	{
		if (NoAmmoSoundEffect)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), NoAmmoSoundEffect, GetActorLocation(), 0.5f, 1.f, 0.f, AttenuationEffect);
		}
	}
}

void AGHWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AGHWeapon, HitScanTrace, COND_SkipOwner);

}
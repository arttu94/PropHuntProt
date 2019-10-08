// Fill out your copyright notice in the Description page of Project Settings.

#include "../Public/GHGhost.h"
#include "../Public/GHProp.h"
#include "../Public/GHHunter.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Engine/World.h"
#include "Public/GHPlayerController.h"

#include "DrawDebugHelpers.h"
#include <EngineGlobals.h>
#include <Runtime/Engine/Classes/Engine/Engine.h>

// Sets default values
AGHGhost::AGHGhost()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
	//StaticMeshComp->SetupAttachment(RootComponent);

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->bUsePawnControlRotation = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	InitialHealth = 250.0f;
}

// Called when the game starts or when spawned
void AGHGhost::BeginPlay()
{
	Super::BeginPlay();
	
	Health = InitialHealth;
}

// Called every frame
void AGHGhost::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/*if(MPC_Grass)
		UKismetMaterialLibrary::SetVectorParameterValue(this, MPC_Grass, "Location", GetActorLocation());*/
}

void AGHGhost::MoveForward(float AxisValue)
{
	if (GetController())
	{
		//FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::X);
		FVector Direction = FRotationMatrix(GetActorRotation()).GetScaledAxis(EAxis::X);
		AddMovementInput(Direction, AxisValue);
	}
}

void AGHGhost::MoveRight(float AxisValue)
{
	if (GetController())
	{
		//FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::Y);
		FVector Direction = FRotationMatrix(GetActorRotation()).GetScaledAxis(EAxis::Y);
		AddMovementInput(Direction, AxisValue);
	}
}

void AGHGhost::SetPitch()
{
	if (GetController())
	{
		Pitch = FMath::Clamp(CameraComp->GetComponentRotation().Pitch, -90.f, 90.f);
	}
}

void AGHGhost::ServerSetPitch_Implementation()
{
	SetPitch();
}

bool AGHGhost::ServerSetPitch_Validate()
{
	return true;
}

void AGHGhost::PitchCamera(float AxisValue)
{
	if (AxisValue != 0)
	{
		if (Role == ROLE_Authority)
		{
			SetPitch();
		}
		else
		{
			ServerSetPitch();
		}
	}

	float SensMultiplier = 1.0f;

	if (GetController())
	{
		SensMultiplier = Cast<AGHPlayerController>(GetController())->MouseSens;
	}

	AddControllerPitchInput(AxisValue * SensMultiplier);
}

void AGHGhost::YawCamera(float AxisValue)
{
	float SensMultiplier = 1.0f;

	if (GetController())
	{
		SensMultiplier = Cast<AGHPlayerController>(GetController())->MouseSens;
	}


	AddControllerYawInput(AxisValue * SensMultiplier);
}

void AGHGhost::PossessObject()
{
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	FHitResult Hit;

	FVector Start = CameraComp->GetComponentLocation();
	FVector End = Start + CameraComp->GetComponentRotation().Vector() * 350.0f;

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Pawn, Params);

	if (bHit)
	{
		APawn* tmp = Cast<APawn>(Hit.Actor.Get());
		if (tmp && !tmp->IsControlled())
		{
			PossessedPawn = tmp;
			if (PossessedPawn)
			{
				if (!Hit.Actor.Get()->GetClass()->IsChildOf(AGHGhost::StaticClass()) & !Hit.Actor.Get()->GetClass()->IsChildOf(AGHHunter::StaticClass()))
				{
					AGHProp* PropPawn = Cast<AGHProp>(PossessedPawn);
					if (Role < ROLE_Authority)
					{
						ServerPossessObject();
					}
					else
					{
						if (GetController())
						{
							PropPawn->PossessingActor = this;
							GetMovementComponent()->Deactivate();
							SetActorHiddenInGame(true);

							SetActorEnableCollision(false);
							MulticastSetActorEnabledCollision(false);

							//SetActorLocation(FVector(-3438.461182, 1580.532104, 528.28064), false);

							GetController()->Possess(PossessedPawn);
						}
					}
				}
			}
		}
	}
}

void AGHGhost::ServerPossessObject_Implementation()
{
	PossessObject();
}

bool AGHGhost::ServerPossessObject_Validate()
{
	return true;
}

void AGHGhost::MulticastSetActorEnabledCollision_Implementation(bool bCol)
{
	//GetCapsuleComponent()->SetCollisionEnabled(ColEnabled);
	SetActorEnableCollision(bCol);
}

void AGHGhost::UnPossessObject()
{
	if (Role == ROLE_Authority)
	{
		AGHProp* PropPawn = Cast<AGHProp>(PossessedPawn);
		if (PropPawn)
		{
			//PropPawn->SetIsPossessed(false);
			PropPawn->SetPossessingActor(nullptr);

			if (PropPawn->GetController())
			{
				GetMovementComponent()->Activate();
				SetActorHiddenInGame(false);
				//SetActorEnableCollision(true);

				PropPawn->GetController()->Possess(this);
			}
		}
	}
	else
	{
		//PropPawn->SetPossessingActor(nullptr);
		ServerUnPossessObject();
	}
}

void AGHGhost::ServerUnPossessObject_Implementation()
{
	UnPossessObject();
}

bool AGHGhost::ServerUnPossessObject_Validate()
{
	return true;
}

float AGHGhost::TakeDamage(float Damage, FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamageCauser)
{
	// Call the base class - this will tell us how much damage to apply  
	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	if (ActualDamage > 0.f)
	{
		Health -= ActualDamage;
		// If the damage depletes our health set our lifespan to zero - which will destroy the actor  
		if (Health <= 0.f)
		{
			//if the player is by himself let the ghost class handle the dying events and logic
			//if (PossessedPawn == nullptr)
			{
				//UnPossessObject();
				bCanBeDamaged = false;
				Die();
				SetLifeSpan(0.01f);
			}
		}
	}

	return ActualDamage;
}

float AGHGhost::GetHealth() const
{
	return Health;
}

// Called to bind functionality to input
void AGHGhost::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AGHGhost::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGHGhost::MoveRight);

	PlayerInputComponent->BindAxis("CameraPitch", this, &AGHGhost::PitchCamera);
	PlayerInputComponent->BindAxis("CameraYaw", this, &AGHGhost::YawCamera);

	PlayerInputComponent->BindAction("Possess", IE_Pressed, this, &AGHGhost::PossessObject);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AGHGhost::Jump);
}

void AGHGhost::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGHGhost, Health);
	DOREPLIFETIME(AGHGhost, Pitch);
}
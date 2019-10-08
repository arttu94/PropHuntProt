// Fill out your copyright notice in the Description page of Project Settings.

#include "../Public/GHProp.h"
#include "../Components/GHPropMovementComponent.h"
#include "../Public/GHGhost.h"
#include "GameFramework/Pawn.h"
#include "Public/GHPlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/Controller.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"

#include "DrawDebugHelpers.h"

#include "Net/UnrealNetwork.h"

#include "EngineGlobals.h"
#include "Engine/Engine.h"

AGHProp::AGHProp()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetSimulatePhysics(true);
	MeshComp->SetNotifyRigidBodyCollision(true);
	MeshComp->OnComponentHit.AddDynamic(this, &AGHProp::OnHit);

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(MeshComp);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	MovingSpeed = 650.0f;
	JumpForce = 450.0f;
	CameraDistScrollMultiplier = 10.0f;

	PawnMovementComp = CreateDefaultSubobject<UGHPropMovementComponent>(TEXT("MovementComp"));
	PawnMovementComp->SetIsReplicated(true);
	PawnMovementComp->SetUpdatedComponent(MeshComp);
	PawnMovementComp->SetMovingSpeed(MovingSpeed);

	bCanStepOnGrass = false;

	bReplicates = true;
	bReplicateMovement = true;
}

void AGHProp::BeginPlay()
{
	Super::BeginPlay();

	PawnMovementComp->SetMovingSpeed(MovingSpeed);

	OriginalLocation = GetActorLocation();
	OriginalRotator = GetActorRotation();
	//bGrounded = true;
}

void AGHProp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsControlled())
	{
		if (bCanStepOnGrass)
		{
			SetVectorParameter();
		}
	}

	if (IsLocallyControlled())
	{
		//keep camera in a non-nausea angle
		if (SpringArmComp->GetUpVector() != FVector::UpVector)
		{
			FRotator Rotation = SpringArmComp->GetComponentRotation();
			Rotation.Roll = 0;
			SpringArmComp->SetWorldRotation(Rotation);
		}

		//Rotate our actor's yaw, which will turn our camera because we're attached to it
		{
			//FRotator NewRotation = GetActorRotation();
			//NewRotation.Yaw += CameraInput.X;
			//SetActorRotation(NewRotation);

			FRotator NewRotation = SpringArmComp->GetComponentRotation();
			NewRotation.Yaw += CameraInput.X;
			//SetActorRotation(NewRotation);
			SpringArmComp->SetWorldRotation(NewRotation);
		}

		//Rotate our camera's pitch, but limit it so we're always looking downward
		{
			FRotator NewRotation = SpringArmComp->GetComponentRotation();
			NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch - CameraInput.Y, -80.0f, -15.0f);
			SpringArmComp->SetWorldRotation(NewRotation);
		}
	}

}

UPawnMovementComponent* AGHProp::GetMovementComponent() const
{
	return PawnMovementComp;
}

void AGHProp::SetPossessingActor(APawn* Actor)
{
	PossessingActor = Actor;
	/*if (Actor != nullptr)
	{
		Cast<AGHGhost>(PossessingActor)->GetMovementComponent()->Deactivate();
		Cast<AGHGhost>(PossessingActor)->MoveIgnoreActorAdd(this);
	}*/
}

//APawn* AGHProp::GetPossessingActor()
//{
//	return PossessingActor;
//}

//void AGHProp::ServerSetPossessingActor_Implementation(APawn* Actor)
//{
//	PossessingActor = Actor;
//}

//bool AGHProp::ServerSetPossessingActor_Validate(APawn* Actor)
//{
//	return true;
//}

void AGHProp::Move(FVector Direction)
{
	if (PawnMovementComp && (PawnMovementComp->UpdatedComponent == RootComponent))
	{
		Direction.Z = 0.f;
		Direction.Normalize();
		PawnMovementComp->AddInputVector(Direction, false);
	}
}

void AGHProp::MoveForward(float AxisValue)
{
	if (AxisValue != 0)
	{
		FVector Direction = SpringArmComp->GetForwardVector();

		if (Role < ROLE_Authority)
		{
			ServerMove(Direction * AxisValue);
		}
		else
		{
			Move(Direction * AxisValue);
		}
	}
}

void AGHProp::MoveRight(float AxisValue)
{
	if (AxisValue != 0)
	{
		FVector Direction = SpringArmComp->GetRightVector();

		if (Role < ROLE_Authority)
		{
			ServerMove(Direction * AxisValue);
		}
		else
		{
			Move(Direction * AxisValue);
		}
	}
}

void AGHProp::ServerMove_Implementation(FVector Direction)
{
	Move(Direction);
}

bool AGHProp::ServerMove_Validate(FVector Direction)
{
	return true;
}

void AGHProp::Jump()
{
	if (Role < ROLE_Authority)
	{
		ServerJump();
	}
	else
	{
		if (bGrounded)
		{
			bGrounded = false;
			MeshComp->AddImpulse(FVector::UpVector * JumpForce, NAME_None, true);
		}
	}
}

void AGHProp::ServerJump_Implementation()
{
	Jump();
}

bool AGHProp::ServerJump_Validate()
{
	return true;
}

void AGHProp::PitchCamera(float AxisValue)
{
	float SensMultiplier = 1.0f;

	if (GetController())
	{
		SensMultiplier = Cast<AGHPlayerController>(GetController())->MouseSens;
	}

	CameraInput.Y = AxisValue * SensMultiplier;
}

void AGHProp::YawCamera(float AxisValue)
{
	float SensMultiplier = 1.0f;

	if (GetController())
	{
		SensMultiplier = Cast<AGHPlayerController>(GetController())->MouseSens;
	}

	CameraInput.X = AxisValue * SensMultiplier;
}

void AGHProp::ZoomCamera(float AxisValue)
{
	if (AxisValue != 0)
	{
		float NewTargetArmLength = 0;
		NewTargetArmLength = SpringArmComp->TargetArmLength + (AxisValue * -CameraDistScrollMultiplier);
		SpringArmComp->TargetArmLength = NewTargetArmLength;
		//SpringArmComp->TargetArmLength = FMath::Clamp(NewTargetArmLength, 100.f, 400.f);
	}
}

void AGHProp::OnHit(UPrimitiveComponent * HitComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, FVector NormalImpulse, const FHitResult & Hit)
{
	if(FVector::DotProduct(Hit.ImpactNormal, FVector::UpVector) > 0.9f)
	{
		bGrounded = true;
	}
}

float AGHProp::TakeDamage(float Damage, FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamageCauser)
{
	// Call the base class - this will tell us how much damage to apply  
	float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	float DamageMultiplier = DamageMultiplier = FMath::Clamp((40 / MeshComp->GetBodyInstance()->GetBodyMass()), 0.25f, 5.0f);

	if (ActualDamage > 0.f)
	{
		ActualDamage = ActualDamage * DamageMultiplier;

		if (IsControlled())
		{
			AGHGhost* ghost = Cast<AGHGhost>(PossessingActor);
			if (ghost)
			{
				ghost->TakeDamage(ActualDamage, DamageEvent, EventInstigator, DamageCauser);

				if (ghost->GetHealth() <= 0.f)
				{
					PossessingActorDied();
					GetController()->UnPossess();
				}
			}
		}
	}

	//DrawDebugString(GetWorld(), FVector::ZeroVector, FString::SanitizeFloat(DamageMultiplier), this, FColor::White, 2.0f, false);

	return ActualDamage;
}

void AGHProp::UnPossessPawn()
{
	//if (PossessingActor)
	{
		AGHGhost* ghost = Cast<AGHGhost>(PossessingActor);
		if (Role < ROLE_Authority)
		{
			ServerUnPossessPawn();
		}
		else
		{
			//GHGhost* ghost = Cast<AGHGhost>(PossessingActor);
			if (ghost)
			{
				ghost->GetMovementComponent()->Activate();
				ghost->SetActorHiddenInGame(false);

				ghost->SetActorEnableCollision(true);
				ghost->MulticastSetActorEnabledCollision(true);

				ghost->SetActorLocation(GetActorLocation() + (FVector::UpVector * 70), false);
				GetController()->Possess(PossessingActor);
				//ghost->PossessedPawn = nullptr;
				PossessingActor = nullptr;
			}
		}
	}
}

void AGHProp::ServerUnPossessPawn_Implementation()
{
	UnPossessPawn();
}

bool AGHProp::ServerUnPossessPawn_Validate()
{
	return true;
}

void AGHProp::SetVectorParameter()
{
	if (Role == ROLE_Authority)
	{
		MulticastSetVectorParameter();
	}
	else
	{
		ServerSetVectorParameter();
	}
}

void AGHProp::ServerSetVectorParameter_Implementation()
{
	//MulticastSetVectorParameter();
	SetVectorParameter();
}

bool AGHProp::ServerSetVectorParameter_Validate()
{
	return true;
}

void AGHProp::MulticastSetVectorParameter_Implementation()
{
	if (MPC_Grass)
		UKismetMaterialLibrary::SetVectorParameterValue(this, MPC_Grass, "Location", GetActorLocation());
}

void AGHProp::PlayRandomSFX()
{
	int index = FMath::RandRange(0, TauntSFX.Num() - 1);
	if (Role < ROLE_Authority)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("AGHProp::PlaySFX"));
		ServerPlayRandomSFX();
	}
	else if(Role == ROLE_Authority)
	{
		MulticastPlayRandomSFX(index);
		//ServerPlayRandomSFX();
	}
}

void AGHProp::ServerPlayRandomSFX_Implementation()
{
	PlayRandomSFX();
}

bool AGHProp::ServerPlayRandomSFX_Validate()
{
	return true;
}

void AGHProp::MulticastPlayRandomSFX_Implementation(int index)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("AGHProp::Multicast_PlaySFX_Implementation"));
	//UGameplayStatics::PlaySound(GetWorld(), TauntSFX[index], Location);
	UGameplayStatics::SpawnSoundAttached(TauntSFX[index], RootComponent, NAME_None, GetActorLocation(), EAttachLocation::KeepWorldPosition, false, 1.0f, 1.0f, 0.0f, SoundAttenuationSettings);
}

void AGHProp::ResetToOriginalLocationAndRotator()
{
	SetActorLocation(OriginalLocation, false, nullptr, ETeleportType::TeleportPhysics);
	SetActorRotation(OriginalRotator, ETeleportType::TeleportPhysics);
}

void AGHProp::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AGHProp::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGHProp::MoveRight);

	PlayerInputComponent->BindAxis("CameraPitch", this, &AGHProp::PitchCamera);
	PlayerInputComponent->BindAxis("CameraYaw", this, &AGHProp::YawCamera);

	PlayerInputComponent->BindAxis("Zoom", this, &AGHProp::ZoomCamera);

	PlayerInputComponent->BindAction("Possess", IE_Pressed, this, &AGHProp::UnPossessPawn);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AGHProp::Jump);

	PlayerInputComponent->BindAction("Taunt", IE_Pressed, this, &AGHProp::PlayRandomSFX);
}

void AGHProp::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGHProp, PossessingActor);
}
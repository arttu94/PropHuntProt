// Fill out your copyright notice in the Description page of Project Settings.

#include "../Public/GHHunter.h"
#include "../Public/GHWeapon.h"
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Public/GHPlayerController.h"
//#include "GHHunter.h"


// Sets default values
AGHHunter::AGHHunter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComp->SetupAttachment(RootComponent);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComp->SetupAttachment(SpringArmComp);

	GetMovementComponent()->SetJumpAllowed(true);
	GetMovementComponent()->NavAgentProps.bCanCrouch = true;

	HatComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HatComponent"));
	HatComp->SetupAttachment(GetMesh(), "head_socket");

	MaskComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MaskComponent"));
	MaskComp->SetupAttachment(GetMesh(), "head_socket");

	WeaponAttachSocketName = "gun_socket";

	//GunComp = CreateDefaultSubobject<AGHWeapon>(TEXT("GunMeshComp"));
	//GunComp->SetupAttachment(GetMesh(), "gun_socket_r");
}

// Called when the game starts or when spawned
void AGHHunter::BeginPlay()
{
	Super::BeginPlay();
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (Role == ROLE_Authority)
	{
		if (PrimaryWeaponClass)
		{
			ActiveWeapon = GetWorld()->SpawnActor<AGHWeapon>(PrimaryWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

			ActiveWeapon->SetOwner(this);
			ActiveWeapon->AttachToComponent((USceneComponent*)GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
		}
	}
}

void AGHHunter::SetPitch()
{
	if (GetController())
	{
		//Pitch = FMath::Clamp(GetController()->GetControlRotation().Pitch, -90.f, 90.f);
		Pitch = FMath::Clamp(CameraComp->GetComponentRotation().Pitch, -90.f, 90.f);
	}
}

void AGHHunter::ServerSetPitch_Implementation()
{
	SetPitch();
}

bool AGHHunter::ServerSetPitch_Validate()
{
	return true;
}

void  AGHHunter::StartJump()
{
	Jump();
	bJumping = true;
}

void  AGHHunter::StopJump()
{
	StopJumping();
}

void  AGHHunter::StartCrouch()
{
	Crouch();
	bCrouching = true;
}

void  AGHHunter::StopCrouch()
{
	UnCrouch();
	bCrouching = false;
}

void AGHHunter::Moveforward(float AxisValue)
{
	if (GetController())
	{
		//FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::X);
		FVector Direction = FRotationMatrix(GetActorRotation()).GetScaledAxis(EAxis::X);
		AddMovementInput(Direction, AxisValue);
	}
}

void AGHHunter::MoveRight(float AxisValue)
{
	if (GetController())
	{
		//FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::Y);
		FVector Direction = FRotationMatrix(GetActorRotation()).GetScaledAxis(EAxis::Y);
		AddMovementInput(Direction, AxisValue);
	}
}

void AGHHunter::PitchCamera(float AxisValue)
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

void AGHHunter::YawCamera(float AxisValue)
{
	float SensMultiplier = 1.0f;

	if (GetController())
	{
		SensMultiplier = Cast<AGHPlayerController>(GetController())->MouseSens;
	}

	AddControllerYawInput(AxisValue * SensMultiplier);
}

void AGHHunter::StartFire()
{
	if (!ActiveWeapon)
		return;

	if (GetController())
	{
		if (!GetController()->IsMoveInputIgnored())
		{
			bFiring = true;
			ActiveWeapon->StartFire();
		}
	}
}

void AGHHunter::StopFire()
{
	if (!ActiveWeapon)
		return;


	if (GetController())
	{
		if (!GetController()->IsMoveInputIgnored())
		{
			bFiring = false;
			ActiveWeapon->StopFire();
		}
	}
}

// Called every frame
void AGHHunter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/*if (MPC_Grass)
		UKismetMaterialLibrary::SetVectorParameterValue(this, MPC_Grass, "Location", GetActorLocation());*/
}

// Called to bind functionality to input
void AGHHunter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AGHHunter::Moveforward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGHHunter::MoveRight);

	PlayerInputComponent->BindAxis("CameraPitch", this, &AGHHunter::PitchCamera);
	PlayerInputComponent->BindAxis("CameraYaw", this, &AGHHunter::YawCamera);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AGHHunter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AGHHunter::StopFire);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AGHHunter::StartJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AGHHunter::StopJump);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AGHHunter::StartCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AGHHunter::StopCrouch);

}

FVector AGHHunter::GetPawnViewLocation() const
{
	if (CameraComp)
	{
		return CameraComp->GetComponentLocation();
	}

	return Super::GetPawnViewLocation();
}

void AGHHunter::DestroyWithWeapons()
{
	ActiveWeapon->Destroy();
	Destroy();
}

void AGHHunter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGHHunter, ActiveWeapon);
	DOREPLIFETIME(AGHHunter, Pitch);
	DOREPLIFETIME(AGHHunter, bFiring);
	DOREPLIFETIME(AGHHunter, bJumping);
	DOREPLIFETIME(AGHHunter, bCrouching);
}
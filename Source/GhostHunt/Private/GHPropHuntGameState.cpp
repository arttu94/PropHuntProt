// Fill out your copyright notice in the Description page of Project Settings.

#include "../Public/GHPropHuntGameState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Public/GHProp.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"


AGHPropHuntGameState::AGHPropHuntGameState()
{

}

/*
void AGHPropHuntGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifeTimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifeTimeProps);

	DOREPLIFETIME(AGHPropHuntGameState, RemainingGhostPlayers);
}
*/

void AGHPropHuntGameState::BeginPlay()
{
	Super::BeginPlay();

	SpawnAllProps();
}

void AGHPropHuntGameState::SpawnAllProps()
{
	//if (Role == ROLE_Authority /*GetWorld()->GetAuthGameMode()*/)
	{
		TArray<AActor*> StaticProps;

		UGameplayStatics::GetAllActorsWithTag(this, "Prop", StaticProps);

		for (size_t i = 0; i < StaticProps.Num(); i++)
		{
			AStaticMeshActor* tmpStaticMeshActor = Cast<AStaticMeshActor>(StaticProps[i]);

			if (tmpStaticMeshActor)
			{
				if (Role == ROLE_Authority)
				{
					FActorSpawnParameters SpawnParams;
					SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

					AGHProp* tmpProp = GetWorld()->SpawnActor<AGHProp>(PropBaseClass, tmpStaticMeshActor->GetTransform(), SpawnParams);
					UStaticMeshComponent* tmpPropMeshComp = Cast<UStaticMeshComponent>(tmpProp->GetComponentByClass(UStaticMeshComponent::StaticClass()));

					tmpPropMeshComp->SetStaticMesh(tmpStaticMeshActor->GetStaticMeshComponent()->GetStaticMesh());

					tmpStaticMeshActor->Destroy();

					MulticastDestroyOriginalMeshActor(tmpStaticMeshActor);
				}
			}
		}
		
		//GetWorld()->ForceGarbageCollection(true);
		//StaticProps.Empty();
	}
}

void AGHPropHuntGameState::MulticastDestroyOriginalMeshActor_Implementation(AActor* ActorToDestroy)
{
	ActorToDestroy->Destroy();
}
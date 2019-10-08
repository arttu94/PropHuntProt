// Fill out your copyright notice in the Description page of Project Settings.

#include "Public/GHMainMenuGameMode.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/CoreDelegates.h"
#include "Kismet/GamePlayStatics.h"
#include "Engine/World.h"

#include "Engine.h"
#include "EngineGlobals.h"

//#include "Runtime/PakFile/Public/IPlatformFilePak.h"

TArray<FString> AGHMainMenuGameMode::GetAllMapNames()
{
	TArray<FString> MapFiles;

	IFileManager::Get().FindFilesRecursive(MapFiles, *FPaths::ProjectContentDir(), TEXT("*.umap"), true, false, false);

	for (int32 i = 0; i < MapFiles.Num(); i++)
	{
		// replace the whole directory string with only the name of the map

		int32 lastSlashIndex = -1;
		if (MapFiles[i].FindLastChar('/', lastSlashIndex))
		{
			FString pureMapName;

			// length - 5 because of the ".umap" suffix
			for (int32 j = lastSlashIndex + 1; j < MapFiles[i].Len() - 5; j++)
			{
				pureMapName.AppendChar(MapFiles[i][j]);
			}

			MapFiles[i] = pureMapName;
		}
	}

	return MapFiles;
}

bool AGHMainMenuGameMode::OpenLevelPak()
{
	return false;
}

void AGHMainMenuGameMode::MountPak(FString PakPath)
{
	if (FCoreDelegates::OnMountPak.IsBound()) 
	{
		//IPlatformFile::FDirectoryVisitor* params;
		FCoreDelegates::OnMountPak.Execute(PakPath, 4, nullptr);
	}
	//return false;
}

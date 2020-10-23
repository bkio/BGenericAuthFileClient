/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "Managers/BManager.h"
#include "Modules/ModuleManager.h"
#include "LevelEditor.h"
#include <random>
#include <string>

void UBKManager::OnConstruct()
{
	FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor").OnMapChanged().AddUObject(this, &UBKManager::OnFirstTimeMapLoaded_Internal);
}

void UBKManager::OnDestruct()
{
	FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor").OnMapChanged().RemoveAll(this);
	bConstructSuccessful = false;
}

bool UBKManager::WasConstructSuccessful()
{
	return bConstructSuccessful;
}

bool UBKManager::IsTickable() const
{
	return bConstructSuccessful;
}

TStatId UBKManager::GetStatId() const
{
	return TStatId();
}

void UBKManager::OnFirstTimeMapLoaded_Internal(UWorld* World, EMapChangeType MapChangeType)
{
	if (MapChangeType == EMapChangeType::NewMap || MapChangeType == EMapChangeType::LoadMap)
	{
		if (bConstructSuccessful) return;
		bConstructSuccessful = true;

		FTimerHandle TimerHandle;
		FTimerDelegate TimerDelegate;

		TWeakObjectPtr<UBKManager> ThisPtr(this);

		//To have the level fully opened.
		TimerDelegate.BindLambda([ThisPtr]()
			{
				if (!ThisPtr.IsValid()) return;
				
				FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor").OnMapChanged().AddUObject(ThisPtr.Get(), &UBKManager::OnNonFirstTimeMapLoaded_Internal);

				ThisPtr->OnFirstTimeMapLoaded();
			});
		World->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 0.01f, false);
	}
}

void UBKManager::OnNonFirstTimeMapLoaded_Internal(UWorld* World, EMapChangeType MapChangeType)
{
	if (MapChangeType == EMapChangeType::NewMap || MapChangeType == EMapChangeType::LoadMap)
	{
		FTimerHandle TimerHandle;
		FTimerDelegate TimerDelegate;

		TWeakObjectPtr<UBKManager> ThisPtr(this);

		//To have the level fully opened.
		TimerDelegate.BindLambda([ThisPtr, World]()
			{
				if (!ThisPtr.IsValid()) return;

				ThisPtr->OnNonFirstTimeMapLoaded();
			});
		World->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 0.01f, false);
	}
	else if (MapChangeType == EMapChangeType::TearDownWorld)
	{
		OnMapIsBeingTornDown();
	}
}

FString UBKManager::GenerateRandomANSIString()
{
	std::string Result("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

	std::random_device RandomDevice;
	std::mt19937 Generator(RandomDevice());

	std::shuffle(Result.begin(), Result.end(), Generator);

	return FString(Result.substr(0, 32).c_str());
}
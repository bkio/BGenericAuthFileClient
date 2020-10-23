/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "Managers/BFileOperationManager.h"

void UBKFileOperationManager::OnConstruct()
{
	Super::OnConstruct();
	OnConstructBP();
}

void UBKFileOperationManager::OnDestruct()
{
	OnDestructBP();
	Super::OnDestruct();
}

void UBKFileOperationManager::OnFirstTimeMapLoaded()
{
	OnFirstTimeMapLoadedBP();
}

void UBKFileOperationManager::OnNonFirstTimeMapLoaded()
{
	OnNonFirstTimeMapLoadedBP();
}

void UBKFileOperationManager::Tick(float DeltaTime)
{
	OnBTick(DeltaTime);
}

bool UBKFileOperationManager::IsTickable() const
{
	return bIsTickable;
}

TStatId UBKFileOperationManager::GetStatId() const
{
	return TStatId();
}

void UBKFileOperationManager::SetIsTickable(bool bTickable)
{
	bIsTickable = bTickable;
}

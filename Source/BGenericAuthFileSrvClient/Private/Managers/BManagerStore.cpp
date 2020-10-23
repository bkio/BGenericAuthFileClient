/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "Managers/BManagerStore.h"
#include "Managers/BNetworkManager.h"
#include "Managers/BNotificationManager.h"
#include "Managers/BViewManager.h"
#include "Managers/BActionManager.h"
#include "Managers/BFileOperationManager.h"
#include "Engine/Blueprint.h"
#include "UObject/ConstructorHelpers.h"

TWeakObjectPtr<UBKManagerStore> UBKManagerStore::WeakSelf = nullptr;

UBKManagerStore::UBKManagerStore() //Use only for being able to call ConstructorHelpers::FObjectFinder
{
	if (HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject)) return;

	static ConstructorHelpers::FObjectFinder<UBlueprint> FileOperationManagerBPFinder(TEXT("/BGenericAuthFileSrvClient/Blueprints/Managers/BP_BK_FileOperationManager.BP_BK_FileOperationManager"));

	if (FileOperationManagerBPFinder.Succeeded())
	{
		BP_FileOperationManager_Asset = FileOperationManagerBPFinder.Object;
	}
}

void UBKManagerStore::Construct()
{
	if (HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject)) return;

	WeakSelf = this;

	NetworkManager = NewObject<UBKNetworkManager>(this);
	((UBKManager*)NetworkManager)->OnConstruct();

	NotificationManager = NewObject<UBKNotificationManager>(this);
	((UBKManager*)NotificationManager)->OnConstruct();

	ViewManager = NewObject<UBKViewManager>(this);
	((UBKManager*)ViewManager)->OnConstruct();

	ActionManager = NewObject<UBKActionManager>(this);
	((UBKManager*)ActionManager)->OnConstruct();

	if (BP_FileOperationManager_Asset.IsValid())
	{
		FileOperationManager = NewObject<UBKFileOperationManager>(this, BP_FileOperationManager_Asset->GeneratedClass);
	}
	else
	{
		FileOperationManager = NewObject<UBKFileOperationManager>(this);
	}
	((UBKManager*)FileOperationManager)->OnConstruct();
}
void UBKManagerStore::BeginDestroy()
{
	if (!HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
	{
		if (NetworkManager != nullptr && NetworkManager->IsValidLowLevel() && !NetworkManager->IsPendingKillOrUnreachable())
			((UBKManager*)NetworkManager)->OnDestruct();
		if (NotificationManager != nullptr && NotificationManager->IsValidLowLevel() && !NotificationManager->IsPendingKillOrUnreachable())
			((UBKManager*)NotificationManager)->OnDestruct();
		if (ViewManager != nullptr && ViewManager->IsValidLowLevel() && !ViewManager->IsPendingKillOrUnreachable())
			((UBKManager*)ViewManager)->OnDestruct();
		if (ActionManager != nullptr && ActionManager->IsValidLowLevel() && !ActionManager->IsPendingKillOrUnreachable())
			((UBKManager*)ActionManager)->OnDestruct();
		if (FileOperationManager != nullptr && FileOperationManager->IsValidLowLevel() && !FileOperationManager->IsPendingKillOrUnreachable())
			((UBKManager*)FileOperationManager)->OnDestruct();
	}
	Super::BeginDestroy();
}

UBKNetworkManager* UBKManagerStore::GetNetworkManager() 
{
	if (WeakSelf.IsValid())
	{
		return WeakSelf->NetworkManager;
	}
	return nullptr;
}

UBKNotificationManager* UBKManagerStore::GetNotificationManager()
{
	if (WeakSelf.IsValid())
	{
		return WeakSelf->NotificationManager;
	}
	return nullptr;
}

UBKViewManager* UBKManagerStore::GetViewManager()
{
	if (WeakSelf.IsValid())
	{
		return WeakSelf->ViewManager;
	}
	return nullptr;
}

UBKActionManager* UBKManagerStore::GetActionManager()
{
	if (WeakSelf.IsValid())
	{
		return WeakSelf->ActionManager;
	}
	return nullptr;
}

UBKFileOperationManager* UBKManagerStore::GetFileOperationManager()
{
	if (WeakSelf.IsValid())
	{
		return WeakSelf->FileOperationManager;
	}
	return nullptr;
}
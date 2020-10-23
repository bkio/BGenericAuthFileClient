/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "BGenericAuthFileSrvClient.h"
#include "Managers/BManagerStore.h"

#define LOCTEXT_NAMESPACE "FBGenericAuthFileSrvClientModule"

TWeakObjectPtr<UBKManagerStore> FBGenericAuthFileSrvClientModule::ManagerStore = nullptr;

void FBGenericAuthFileSrvClientModule::SetupManagerStore() //Called from startup
{
	ManagerStore = NewObject<UBKManagerStore>(GetTransientPackage(), NAME_None, RF_Public | RF_Standalone | RF_MarkAsRootSet);
	ManagerStore->ClearFlags(RF_Transactional);
	ManagerStore->Construct();
}

void FBGenericAuthFileSrvClientModule::StartupModule()
{
	SetupManagerStore();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FBGenericAuthFileSrvClientModule, BGenericAuthFileSrvClient)
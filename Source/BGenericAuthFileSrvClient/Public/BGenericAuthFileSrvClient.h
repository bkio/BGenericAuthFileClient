/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FBGenericAuthFileSrvClientModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	//Note: On ShutdownModule; all UObjects are already destroyed.

private:
	static TWeakObjectPtr<class UBKManagerStore> ManagerStore;
	void SetupManagerStore();
};
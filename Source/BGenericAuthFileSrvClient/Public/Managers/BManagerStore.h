/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "BManagerStore.generated.h"

/**
 * 
 */
UCLASS()
class BGENERICAUTHFILESRVCLIENT_API UBKManagerStore : public UObject
{
	GENERATED_BODY()

public:
	UBKManagerStore();

	UFUNCTION(BlueprintPure, Category = "BGenericAuthFileSrvClient|Managers")
	static class UBKNetworkManager* GetNetworkManager();

	UFUNCTION(BlueprintPure, Category = "BGenericAuthFileSrvClient|Managers")
	static class UBKNotificationManager* GetNotificationManager();

	UFUNCTION(BlueprintPure, Category = "BGenericAuthFileSrvClient|Managers")
	static class UBKViewManager* GetViewManager();

	UFUNCTION(BlueprintPure, Category = "BGenericAuthFileSrvClient|Managers")
	static class UBKActionManager* GetActionManager();

	UFUNCTION(BlueprintPure, Category = "BGenericAuthFileSrvClient|Managers")
	static class UBKFileOperationManager* GetFileOperationManager();

	friend class FBGenericAuthFileSrvClientModule; //Only this should have access to Construct function.

	virtual void BeginDestroy() override;

private:
	void Construct();

	static TWeakObjectPtr<UBKManagerStore> WeakSelf;

	UPROPERTY()
	class UBKNetworkManager* NetworkManager;

	UPROPERTY()
	class UBKNotificationManager* NotificationManager;

	UPROPERTY()
	class UBKActionManager* ActionManager;
	
	UPROPERTY()
	class UBKViewManager* ViewManager;

	UPROPERTY()
	class UBKFileOperationManager* FileOperationManager;

	TWeakObjectPtr<class UBlueprint> BP_FileOperationManager_Asset;
};
/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "Managers/BManager.h"
#include "BNotificationManager.generated.h"

/**
 * 
 */
UCLASS()
class BGENERICAUTHFILESRVCLIENT_API UBKNotificationManager : public UBKManager
{
	GENERATED_BODY()

protected:
	virtual void OnConstruct() override;
	virtual void OnDestruct() override;

public:
	void SendNotification(const ANSICHAR* Message, float ExpireDuration = 4.0f);
	void SendNotification(const TCHAR* Message, float ExpireDuration = 4.0f);
	void SendNotification(const FText& Message, float ExpireDuration = 4.0f);

    UFUNCTION(BlueprintCallable, Category = "BGenericAuthFileSrvClient|Managers|Notification Manager")
	void SendNotification(const FString& Message, float ExpireDuration = 4.0f);
};
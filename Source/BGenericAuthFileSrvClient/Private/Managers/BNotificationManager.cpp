/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "Managers/BNotificationManager.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

void UBKNotificationManager::OnConstruct()
{
	Super::OnConstruct();
}

void UBKNotificationManager::OnDestruct()
{
	//
	Super::OnDestruct();
}

void UBKNotificationManager::SendNotification(const ANSICHAR* Message, float ExpireDuration)
{
	SendNotification(ANSI_TO_TCHAR(Message), ExpireDuration);
}

void UBKNotificationManager::SendNotification(const TCHAR* Message, float ExpireDuration)
{
	FText NotificationMessage = FText::FromString(Message);
	SendNotification(NotificationMessage, ExpireDuration);
}

void UBKNotificationManager::SendNotification(const FString& Message, float ExpireDuration)
{
	FText NotificationMessage = FText::FromString(Message);
	SendNotification(NotificationMessage, ExpireDuration);
}

void UBKNotificationManager::SendNotification(const FText& Message, float ExpireDuration)
{
	FNotificationInfo Info(Message);
	Info.ExpireDuration = ExpireDuration;
	FSlateNotificationManager::Get().AddNotification(Info);
}
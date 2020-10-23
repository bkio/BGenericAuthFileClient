/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "Network/BLoginBrowserListener.h"
#include "BHttpServerDefines.h"
#include "Managers/BManagerStore.h"
#include "Managers/BNetworkManager.h"
#include "GenericPlatform/GenericPlatformHttp.h"

BKLoginBrowserListener::~BKLoginBrowserListener()
{
	if (ProcessCompletedDelegateHandle.IsValid())
	{
		OnResponseCompleted.Remove(ProcessCompletedDelegateHandle);
	}
}

void BKLoginBrowserListener::ProcessRequest(const FGuid& RequestUniqueId, const FString& RequestData)
{
	bool bLoggedIn = false;

	FString Message;

	if (RequestData.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to log in - No request data received from server, please try again."));
	}

	else if (RequestData.Contains("error"))
	{
		// Create error message
		Message = FGenericPlatformHttp::UrlDecode(RequestData);
		Message = Message.Replace(TEXT("error_message="), TEXT(""));
		Message = Message.Replace(TEXT("+"), TEXT(" "));
		UE_LOG(LogTemp, Error, TEXT("%s"), *Message);
		bLoggedIn = false;
	}

	else if (RequestData.Contains("user"))
	{
		// Create and store userId and userToken
		FString UserId;
		FString UserToken;
		static const FString Splitter = "&";

		RequestData.Split(Splitter, &UserId, &UserToken);
		UserId = UserId.Replace(TEXT("user_id="), TEXT(""));
		UserToken = UserToken.Replace(TEXT("token="), TEXT(""));
		UserToken = UserToken.Replace(TEXT("+"), TEXT(" "));
		UBKManagerStore::GetNetworkManager()->StoreNetworkUser(UserId, UserToken);
		bLoggedIn = true;
	}
	
	ProcessRequestFinish(RequestUniqueId, "<html><head><script type=\"text/javascript\">window.open('', '_self', ''); window.close();</script></head></html>");
	ProcessCompletedDelegateHandle = OnResponseCompleted.AddLambda(
		[bLoggedIn]
		(const FGuid& Id)
		{
			UBKManagerStore::GetNetworkManager()->BroadcastLoginStatus(bLoggedIn);
		});
}

void BKLoginBrowserListener::OpenBrowser(const FString& Url)
{
	FPlatformProcess::LaunchURL(*Url, NULL, NULL);
}
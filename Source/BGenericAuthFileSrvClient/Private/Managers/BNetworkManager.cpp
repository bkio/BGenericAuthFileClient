/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "Managers/BNetworkManager.h"
#include "Managers/BManagerStore.h"
#include "Managers/BNotificationManager.h"

#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/SecureHash.h"

#include "Network/BLoginBrowserListener.h"
#include "Network/BHTTPWrapper.h"
#include "Network/BCancellationToken.h"
#include "BHttpServer.h"

#include "UI/BWidget.h"

#include "JsonUtilities.h"
#include "UnrealEd.h"

namespace BFetchApiEndpoint
{
	#define BAPIENDPOINTBASE_INI_PATH FString()

	FString BProcess::CachedBApiEndpointBase = "";
	FString BProcess::BApiEndpointBaseIniPath = FPaths::ProjectConfigDir() + "BApiEndpointBase.ini";

	FString BProcess::GetBApiEndpointBase()
	{
		if (CachedBApiEndpointBase.Len() == 0)
		{
			if (!FPaths::FileExists(BApiEndpointBaseIniPath) || !FFileHelper::LoadFileToString(BApiEndpointBaseIniPath, *CachedBApiEndpointBase))
			{
				CachedBApiEndpointBase = "http://localhost/";
			}
		}
		return CachedBApiEndpointBase;
	}
}

void UBKNetworkManager::OnConstruct()
{
	Super::OnConstruct();
	if (!LoadNetworkUser())
	{
		CurrentNetworkUser.Id = "";
		CurrentNetworkUser.Token = "";
		CurrentNetworkUser.UserName = "";
		CurrentNetworkUser.Email = "";
		UE_LOG(LogTemp, Warning, TEXT("Failed to load previous network user"));
	}
}
void UBKNetworkManager::OnDestruct()
{
	//
	Super::OnDestruct();
}

void UBKNetworkManager::OnMapIsBeingTornDown()
{
}

bool UBKNetworkManager::StartLoginProcedure(bool bUseCancellationToken, TSharedPtr<BKCancellationTokenWeakWrapper> CancellationToken)
{
	if (!CurrentNetworkUser.Token.IsEmpty())
	{
		FString CheckRefreshTokenURL = BASE_PATH + "auth/login/azure/token_refresh";

		TWeakObjectPtr<UBKNetworkManager> ThisPtr(this);

		auto OnFailure = [ThisPtr]
		(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message, int32 ErrorCode)
		{
			if (ThisPtr.IsValid())
			{
				UE_LOG(LogTemp, Warning, TEXT("Token refresh attempt failed with %d: %s"), ErrorCode, *Message);
				if (!ThisPtr->StartLoginProcedure_Internal())
				{
					ThisPtr->OnLoginFailed.Broadcast(TEXT("Could not start http server; check application permissions."));
				}
				ThisPtr->FlagHttpWrapperForDeletion(HttpWrapper);
			}
		};
		auto OnSuccess = [ThisPtr]
		(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message)
		{
			if (ThisPtr.IsValid() && HttpWrapper.IsValid())
			{
				TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
				HttpWrapper.Pin()->GetResponseObject(JsonObject);

				FString NewToken;
				if (!JsonObject->TryGetStringField("token", NewToken))
					UE_LOG(LogTemp, Warning, TEXT("Error while retrieving user name from http response."));
				NewToken = NewToken.Replace(TEXT("+"), TEXT(" "));

				FString NewUserId;
				if (!JsonObject->TryGetStringField("userId", NewUserId))
					UE_LOG(LogTemp, Warning, TEXT("Error while retrieving userId from http response."));

				FString SuccessStatus;
				if (!JsonObject->TryGetStringField("status", SuccessStatus))
					UE_LOG(LogTemp, Warning, TEXT("Error while retrieving success status from http response."));
				UE_LOG(LogTemp, Log, TEXT("Token check/refresh attempt succeeded with: %s"), *SuccessStatus);

				ThisPtr->StoreNetworkUser(NewUserId, NewToken);

				ThisPtr->FlagHttpWrapperForDeletion(HttpWrapper);
			}
		};

		FString Summary = CalculateRequestSummary(CheckRefreshTokenURL, EBKHTTPRequestType::POST, BKHTTPWrapper::EmptyJsonObject, bUseCancellationToken);
		TWeakPtr<BKHTTPWrapper> Existing = CheckForAnIdenticalRequestAndBindIfExists(Summary, OnFailure, OnSuccess, bUseCancellationToken, CancellationToken);
		if (!Existing.IsValid())
		{
			NewNonExistentHttpWrapperPrechecked(Summary, CheckRefreshTokenURL, OnFailure, OnSuccess, bUseCancellationToken, CancellationToken).Pin()->Post(BKHTTPWrapper::EmptyJsonObject);
		}
	}
	return StartLoginProcedure_Internal();
}
bool UBKNetworkManager::StartLoginProcedure_Internal()
{
	Server = MakeShareable(new BHttpServer());
	if (!Server->StartServer("127.0.0.1:56789")) 
	{
		return false;
	}

	Listener = MakeShareable(new BKLoginBrowserListener());
	Listener->SetHeader(TEXT("Content-Type"), TEXT("text/html"));
	Server->AddHandler(Listener, TEXT("/"));

	FString BrowserURL = BASE_PATH + "auth/login/azure";
	if (!CurrentNetworkUser.Token.IsEmpty())
	{
		BrowserURL += "?existing_token=" + FGenericPlatformHttp::UrlEncode(CurrentNetworkUser.Token);
	}
	Listener->OpenBrowser(BrowserURL);
	return true;
}

void UBKNetworkManager::StopLoginProcedure()
{
	if (Server.IsValid())
	{
		Server->RemoveHandler(TEXT("/"));
		Server->StopServer();
	}
	Server.Reset();
	Listener.Reset();
}

void UBKNetworkManager::Logout()
{
	StopLoginProcedure();
	DeleteCurrentNetworkUserData();
	UBKManagerStore::GetNotificationManager()->SendNotification("Logged out");
}

void UBKNetworkManager::BroadcastLoginStatus(bool bLoggedIn)
{
	//If there was another login attempt during cancellation recognizition of the caller
	//OnLoginFailed should not be called and StopLoginProcedure should not be performed.

	FString Message;
	if (bLoggedIn)
	{
		Message = "Login succeeded";
		OnLoginSucceed.Broadcast(Message);
	}
	else
	{
		Message = "Login failed";
		OnLoginFailed.Broadcast(Message);
	}

	// Stop login server 
	StopLoginProcedure();
}

//Step 1
FString UBKNetworkManager::CalculateRequestSummary(
	const FString& Url,
	EBKHTTPRequestType RequestType,
	const TSharedPtr<FJsonObject>& JsonContent,
	bool bUseCancellationToken)
{
	FString StringifiedContent;
	if (JsonContent.IsValid())
	{
		TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<>::Create(&StringifiedContent);
		FJsonSerializer::Serialize(JsonContent.ToSharedRef(), JsonWriter);
	}
	else StringifiedContent = "NULL";

	return FMD5::HashAnsiString(*FString::Printf(TEXT("%s-%d-%s-%d"), *Url, RequestType, *StringifiedContent, bUseCancellationToken));
}
FString UBKNetworkManager::CalculateRequestSummary(
	const FString& Url,
	EBKHTTPRequestType RequestType,
	const FString& Stringified,
	bool bUseCancellationToken)
{
	return FMD5::HashAnsiString(*FString::Printf(TEXT("%s-%d-%s-%d"), *Url, RequestType, *Stringified, bUseCancellationToken));
}

//Step 2
TWeakPtr<BKHTTPWrapper> UBKNetworkManager::CheckForAnIdenticalRequestAndBindIfExists(
	const FString& RequestSummary,
	TFunction<void(TWeakPtr<BKHTTPWrapper>, const FString&, int32)> OnFailure,
	TFunction<void(TWeakPtr<BKHTTPWrapper>, const FString&)> OnSuccess,
	bool bUseCancellationToken,
	TSharedPtr<BKCancellationTokenWeakWrapper> WithCancellationToken)
{
	if (RequestSummaryHttpWrapperMap.Contains(RequestSummary))
	{
		TWeakPtr<BKHTTPWrapper> HttpWrapper = RequestSummaryHttpWrapperMap[RequestSummary];

		if (HttpWrapper.IsValid() && HttpWrapper.IsValid())
		{
			auto StrongHttpWrapper = HttpWrapper.Pin();
			
			if (StrongHttpWrapper->LastErrorCode != 0)
			{
				RequestSummaryHttpWrapperMap.Remove(RequestSummary);
				return nullptr;
			}
			
			StrongHttpWrapper->AppendBindings(OnFailure, OnSuccess);
			if (bUseCancellationToken) StrongHttpWrapper->AppendCancellationToken(WithCancellationToken);
			
			TWeakObjectPtr<UBKNetworkManager> ThisPtr(this);

			StrongHttpWrapper->OnHttpRequestFailed.AddLambda(
				[ThisPtr]
				(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message, int32 ErrorCode)
				{
					if (ThisPtr.IsValid() && HttpWrapper.IsValid())
					{
						UE_LOG(LogTemp, Log, TEXT("Url: %s Http request failed with the following code: %d message: %s"), *HttpWrapper.Pin()->RequestUrl, ErrorCode, *Message);
						ThisPtr->FlagHttpWrapperForDeletion(HttpWrapper);
					}
				});

			StrongHttpWrapper->OnHttpRequestSucceed.AddLambda(
				[ThisPtr]
				(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message)
				{
					if (ThisPtr.IsValid())
					{
						ThisPtr->FlagHttpWrapperForDeletion(HttpWrapper);
					}
				});

			return HttpWrapper;
		}
	}
	return nullptr;
}

//Step 3
TWeakPtr<BKHTTPWrapper> UBKNetworkManager::NewNonExistentHttpWrapperPrechecked(
	const FString& RequestSummary,
	const FString& Url, 
	TFunction<void(TWeakPtr<BKHTTPWrapper>, const FString&, int32)> OnFailure,
	TFunction<void(TWeakPtr<BKHTTPWrapper>, const FString&)> OnSuccess,
	bool bUseCancellationToken,
	TSharedPtr<BKCancellationTokenWeakWrapper> CancellationToken)
{
	TSharedPtr<BKHTTPWrapper> NewWrapper = MakeShareable<BKHTTPWrapper>(new BKHTTPWrapper());
	NewWrapper->Initialize(NewWrapper, RequestSummary, Url, OnFailure, OnSuccess);

	if (bUseCancellationToken) NewWrapper->AppendCancellationToken(CancellationToken);
	HttpWrappers.Add(NewWrapper);
	RequestSummaryHttpWrapperMap.Add(RequestSummary, NewWrapper);

	TWeakObjectPtr<UBKNetworkManager> ThisPtr(this);

	NewWrapper->OnHttpRequestFailed.AddLambda(
		[ThisPtr]
		(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message, int32 ErrorCode)
		{
			if (ThisPtr.IsValid() && HttpWrapper.IsValid())
			{
				UE_LOG(LogTemp, Log, TEXT("Url: %s Http request failed with the following code: %d message: %s"), *HttpWrapper.Pin()->RequestUrl, ErrorCode, *Message);
				ThisPtr->FlagHttpWrapperForDeletion(HttpWrapper);
			}
		});

	NewWrapper->OnHttpRequestSucceed.AddLambda(
		[ThisPtr]
		(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message)
		{
			if (ThisPtr.IsValid())
			{
				ThisPtr->FlagHttpWrapperForDeletion(HttpWrapper);
			}
		});

	return NewWrapper;
}

TWeakPtr<BKHTTPWrapper> UBKNetworkManager::Get(
	const FString& Url, 
	TFunction<void(TWeakPtr<BKHTTPWrapper>, const FString&, int32)> OnFailure,
	TFunction<void(TWeakPtr<BKHTTPWrapper>, const FString&)> OnSuccess,
	bool bUseCancellationToken,
	TSharedPtr<BKCancellationTokenWeakWrapper> WithCancellationToken)
{
	FString Summary = CalculateRequestSummary(Url, EBKHTTPRequestType::GET, nullptr, bUseCancellationToken);
	TWeakPtr<BKHTTPWrapper> HttpWrapper = CheckForAnIdenticalRequestAndBindIfExists(Summary, OnFailure, OnSuccess, bUseCancellationToken, WithCancellationToken);
	
	if (!HttpWrapper.IsValid())
	{
		HttpWrapper = NewNonExistentHttpWrapperPrechecked(Summary, Url, OnFailure, OnSuccess, bUseCancellationToken, WithCancellationToken);
		HttpWrapper.Pin()->Get();
	}
	return HttpWrapper;
}

TWeakPtr<BKHTTPWrapper> UBKNetworkManager::Delete(
	const FString& Url, 
	TFunction<void(TWeakPtr<BKHTTPWrapper>, const FString&, int32)> OnFailure,
	TFunction<void(TWeakPtr<BKHTTPWrapper>, const FString&)> OnSuccess,
	bool bUseCancellationToken,
	TSharedPtr<BKCancellationTokenWeakWrapper> WithCancellationToken)
{
	FString Summary = CalculateRequestSummary(Url, EBKHTTPRequestType::DELETE, nullptr, bUseCancellationToken);
	TWeakPtr<BKHTTPWrapper> HttpWrapper = CheckForAnIdenticalRequestAndBindIfExists(Summary, OnFailure, OnSuccess, bUseCancellationToken, WithCancellationToken);

	if (!HttpWrapper.IsValid())
	{
		HttpWrapper = NewNonExistentHttpWrapperPrechecked(Summary, Url, OnFailure, OnSuccess, bUseCancellationToken, WithCancellationToken);
		HttpWrapper.Pin()->Delete();
	}
	return HttpWrapper;
}

TWeakPtr<BKHTTPWrapper> UBKNetworkManager::Post(
	const FString& Url, 
	const FString& StringContent, 
	TFunction<void(TWeakPtr<BKHTTPWrapper>, const FString&, int32)> OnFailure,
	TFunction<void(TWeakPtr<BKHTTPWrapper>, const FString&)> OnSuccess,
	bool bUseCancellationToken,
	TSharedPtr<BKCancellationTokenWeakWrapper> WithCancellationToken)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(StringContent);

	if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("POST failed! Error while deserializing string data: %s."), *StringContent);
		return nullptr;
	}

	FString Summary = CalculateRequestSummary(Url, EBKHTTPRequestType::POST, StringContent, bUseCancellationToken);
	TWeakPtr<BKHTTPWrapper> HttpWrapper = CheckForAnIdenticalRequestAndBindIfExists(Summary, OnFailure, OnSuccess, bUseCancellationToken, WithCancellationToken);

	if (!HttpWrapper.IsValid())
	{
		HttpWrapper = NewNonExistentHttpWrapperPrechecked(Summary, Url, OnFailure, OnSuccess, bUseCancellationToken, WithCancellationToken);
		HttpWrapper.Pin()->Post(JsonObject);
	}
	return HttpWrapper;
}

TWeakPtr<BKHTTPWrapper> UBKNetworkManager::Post(
	const FString& Url, 
	const TSharedPtr<FJsonObject>& JsonContent, 
	TFunction<void(TWeakPtr<BKHTTPWrapper>, const FString&, int32)> OnFailure,
	TFunction<void(TWeakPtr<BKHTTPWrapper>, const FString&)> OnSuccess,
	bool bUseCancellationToken,
	TSharedPtr<BKCancellationTokenWeakWrapper> WithCancellationToken)
{
	FString Summary = CalculateRequestSummary(Url, EBKHTTPRequestType::POST, JsonContent, bUseCancellationToken);
	TWeakPtr<BKHTTPWrapper> HttpWrapper = CheckForAnIdenticalRequestAndBindIfExists(Summary, OnFailure, OnSuccess, bUseCancellationToken, WithCancellationToken);

	if (!HttpWrapper.IsValid())
	{
		HttpWrapper = NewNonExistentHttpWrapperPrechecked(Summary, Url, OnFailure, OnSuccess, bUseCancellationToken, WithCancellationToken);
		HttpWrapper.Pin()->Post(JsonContent);
	}
	return HttpWrapper;
}

TWeakPtr<BKHTTPWrapper> UBKNetworkManager::Put(
	const FString& Url, 
	const FString& StringContent, 
	TFunction<void(TWeakPtr<BKHTTPWrapper>, const FString&, int32)> OnFailure,
	TFunction<void(TWeakPtr<BKHTTPWrapper>, const FString&)> OnSuccess,
	bool bUseCancellationToken,
	TSharedPtr<BKCancellationTokenWeakWrapper> WithCancellationToken)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(StringContent);

	if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("PUT failed! Error while deserializing file data: %s."), *StringContent);
		return nullptr;
	}

	FString Summary = CalculateRequestSummary(Url, EBKHTTPRequestType::PUT, StringContent, bUseCancellationToken);
	TWeakPtr<BKHTTPWrapper> HttpWrapper = CheckForAnIdenticalRequestAndBindIfExists(Summary, OnFailure, OnSuccess, bUseCancellationToken, WithCancellationToken);

	if (!HttpWrapper.IsValid())
	{
		HttpWrapper = NewNonExistentHttpWrapperPrechecked(Summary, Url, OnFailure, OnSuccess, bUseCancellationToken, WithCancellationToken);
		HttpWrapper.Pin()->Put(JsonObject);
	}
	return HttpWrapper;
}

TWeakPtr<BKHTTPWrapper> UBKNetworkManager::Put(
	const FString& Url, 
	const TSharedPtr<FJsonObject>& JsonContent, 
	TFunction<void(TWeakPtr<BKHTTPWrapper>, const FString&, int32)> OnFailure,
	TFunction<void(TWeakPtr<BKHTTPWrapper>, const FString&)> OnSuccess,
	bool bUseCancellationToken,
	TSharedPtr<BKCancellationTokenWeakWrapper> WithCancellationToken)
{
	FString Summary = CalculateRequestSummary(Url, EBKHTTPRequestType::PUT, JsonContent, bUseCancellationToken);
	TWeakPtr<BKHTTPWrapper> HttpWrapper = CheckForAnIdenticalRequestAndBindIfExists(Summary, OnFailure, OnSuccess, bUseCancellationToken, WithCancellationToken);

	if (!HttpWrapper.IsValid())
	{
		HttpWrapper = NewNonExistentHttpWrapperPrechecked(Summary, Url, OnFailure, OnSuccess, bUseCancellationToken, WithCancellationToken);
		HttpWrapper.Pin()->Put(JsonContent);
	}
	return HttpWrapper;
}

bool UBKNetworkManager::ValidateUser(bool bUseCancellationToken, TSharedPtr<BKCancellationTokenWeakWrapper> CancellationToken)
{
	if (CurrentNetworkUser.Id.Len() == 0) return false;

	FString Url = BASE_PATH;
	Url.Append(END_POINT_AUTH_USER + CurrentNetworkUser.Id);

	TWeakObjectPtr<UBKNetworkManager> ThisPtr(this);
	auto OnFailure = [ThisPtr](TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message, int32 ErrorCode)
	{
		if (ThisPtr.IsValid())
		{
			ThisPtr->NotifyAuthenticatedWidgetObservers(false);
			ThisPtr->FlagHttpWrapperForDeletion(HttpWrapper);
		}
	};
	auto OnSuccess = [ThisPtr]
	(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message)
	{
		if (ThisPtr.IsValid() && HttpWrapper.IsValid())
		{
			TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
			HttpWrapper.Pin()->GetResponseObject(JsonObject);

			if (!JsonObject->TryGetStringField("userName", ThisPtr->CurrentNetworkUser.UserName))
				UE_LOG(LogTemp, Warning, TEXT("Error while retrieving user name from http response."));

			if (!JsonObject->TryGetStringField("userEmail", ThisPtr->CurrentNetworkUser.Email))
				UE_LOG(LogTemp, Warning, TEXT("Error while retrieving email from http response."));

			ThisPtr->NotifyAuthenticatedWidgetObservers(true);
			ThisPtr->FlagHttpWrapperForDeletion(HttpWrapper);
		}
	};

	FString Summary = CalculateRequestSummary(Url, EBKHTTPRequestType::GET, nullptr, bUseCancellationToken);
	TWeakPtr<BKHTTPWrapper> ExistingWrapper = CheckForAnIdenticalRequestAndBindIfExists(Summary, OnFailure, OnSuccess, bUseCancellationToken, CancellationToken);

	if (!ExistingWrapper.IsValid())
	{
		NewNonExistentHttpWrapperPrechecked(Summary, Url, OnFailure, OnSuccess, bUseCancellationToken, CancellationToken).Pin()->Get();
	}

	return true;
}

void UBKNetworkManager::DeleteCurrentNetworkUserData()
{
	StoreNetworkUser("", "");
}

bool UBKNetworkManager::GetCurrentUserId(FString& UserId)
{
	UserId = CurrentNetworkUser.Id;
	return UserId.Len() > 0;
}

bool UBKNetworkManager::GetCurrentUserToken(FString& Token)
{
	Token = CurrentNetworkUser.Token;
	return Token.Len() > 0;
}

bool UBKNetworkManager::GetCurrentUserName(FString& UserName)
{
	UserName = CurrentNetworkUser.UserName;
	return UserName.Len() > 0;
}

bool UBKNetworkManager::GetCurrentUserEmail(FString& UserEmail)
{
	UserEmail = CurrentNetworkUser.Email;
	return UserEmail.Len() > 0;
}

void UBKNetworkManager::RegisterAuthenticatedWidgetObserver(UBKViewWidget* Observer)
{
	if (!Observer || !Observer->IsValidLowLevel() || Observer->IsPendingKillOrUnreachable()) return;

	for (TWeakObjectPtr<UBKViewWidget> Current : ObserverWidgets)
	{
		if (Current.IsValid() && Current.Get() == Observer)
		{
			//Already exists
			return;
		}
	}

	ObserverWidgets.Add(Observer);
	bCanTick = true;
}

void UBKNetworkManager::UnregisterAuthenticatedWidgetObserver(UBKViewWidget* Observer)
{
	if (!Observer) return;
	//Do not access to any content of Observer here. It might be pending kill.

	int32 i = 0;
	for (TWeakObjectPtr<UBKViewWidget> Current : ObserverWidgets)
	{
		if (Current.IsValid(true/*bEvenIfPendingKill*/) && Current.GetEvenIfUnreachable() == Observer)
		{
			ObserverWidgets.RemoveAt(i);
			break;
		}
		i++;
	}
}

void UBKNetworkManager::UnregisterAllAuthenticatedWidgetObservers()
{
	ObserverWidgets.Empty();
}

TArray<TWeakObjectPtr<UBKViewWidget>> UBKNetworkManager::GetObserverWidgetsCopy() const
{
	return ObserverWidgets;
}

void UBKNetworkManager::Tick(float DeltaTime)
{
	if (ObserverWidgets.Num() == 0)
	{
		bCanTick = false;
		return;
	}
}

void UBKNetworkManager::StoreNetworkUser(const FString& UserId, const FString& UserToken)
{
	FString SavePath = SAVE_FILE_PATH;
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	TSharedPtr<FJsonObject> NetworkUserJsonObject = MakeShareable(new FJsonObject);

	NetworkUserJsonObject->SetStringField("id", UserId);
	NetworkUserJsonObject->SetStringField("token", UserToken);
	JsonObject->SetObjectField("network_user", NetworkUserJsonObject);

	FString OutputString;
	auto Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	FFileHelper::SaveStringToFile(OutputString, *SavePath);

	CurrentNetworkUser.Id = UserId;
	CurrentNetworkUser.Token = UserToken;
	BKHTTPWrapper::TokenCached = CurrentNetworkUser.Token;
}

bool UBKNetworkManager::LoadNetworkUser()
{
	const FString JsonFilePath = SAVE_FILE_PATH;
	FString JsonString;

	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*JsonFilePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("Error while loading file: %s  - File does not exist."), *JsonFilePath);
		return false;
	}

	FFileHelper::LoadFileToString(JsonString, *JsonFilePath);
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);

	if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Error while deserializing file data: %s."), *JsonFilePath);
		return false;
	}

	const TSharedPtr<FJsonObject>* NetworkUserObject;

	if (!JsonObject->TryGetObjectField("network_user", NetworkUserObject))
	{
		UE_LOG(LogTemp, Warning, TEXT("Error while loading file data: %s  - Could not read ObjectField 'network_user'."), *JsonFilePath);
		return false;
	}

	if (!(*NetworkUserObject)->TryGetStringField("id", CurrentNetworkUser.Id))
	{
		UE_LOG(LogTemp, Warning, TEXT("Error while loading file data: %s  - Could not read 'id'."), *JsonFilePath);
		return false;
	}

	if (!(*NetworkUserObject)->TryGetStringField("token", CurrentNetworkUser.Token))
	{
		UE_LOG(LogTemp, Warning, TEXT("Error while loading file data: %s  - Could not read 'token'."), *JsonFilePath);
		return false;
	}

	BKHTTPWrapper::TokenCached = CurrentNetworkUser.Token;

	return true;
}

void UBKNetworkManager::FlagHttpWrapperForDeletion(TWeakPtr<BKHTTPWrapper> HttpWrapper)
{
	//This delay is due to multiple lambda bindings to the OnHttpRequestSucceed and OnHttpRequestFailed.
	//We do not know which callback is called first; this or others; for others to be able to access BKHTTPWrapper reference
	//A delay is needed; otherwise it will be garbage collected; because GET, POST, DELETE, PUT functions return weak ptr.
	if (UWorld* World = GEditor->GetEditorWorldContext().World())
	{
		if (World->IsValidLowLevel() && !World->IsPendingKillOrUnreachable())
		{
			TWeakObjectPtr<UBKNetworkManager> ThisPtr(this);

			FTimerHandle TimerHandle;
			FTimerDelegate TimerDelegate;
			TimerDelegate.BindLambda([ThisPtr, HttpWrapper]()
			{
				if (ThisPtr.IsValid() && HttpWrapper.IsValid())
				{
					auto StrongHttpWrapper = HttpWrapper.Pin();
					ThisPtr->RequestSummaryHttpWrapperMap.Remove(StrongHttpWrapper->RequestSummary);
					ThisPtr->HttpWrappers.Remove(StrongHttpWrapper);
				}
			});
			World->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 0.01f, false);
			return;
		}
	}
	if (HttpWrapper.IsValid())
	{
		auto StrongHttpWrapper = HttpWrapper.Pin();
		RequestSummaryHttpWrapperMap.Remove(StrongHttpWrapper->RequestSummary);
		HttpWrappers.Remove(StrongHttpWrapper);
	}
}

void UBKNetworkManager::NotifyAuthenticatedWidgetObservers(bool bAuthenticated)
{
	for (int32 i = ObserverWidgets.Num() - 1; i >= 0; i--)
	{
		if (!ObserverWidgets[i].IsValid())
			ObserverWidgets.RemoveAt(i);
		else
			ObserverWidgets[i]->OnAuthenticationStatusChanged(bAuthenticated);
	}
}
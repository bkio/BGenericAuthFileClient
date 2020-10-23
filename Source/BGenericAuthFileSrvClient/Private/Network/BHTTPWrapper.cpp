/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "Network/BHTTPWrapper.h"
#include "Network/BCancellationToken.h"
#include "JsonUtilities.h"

const TSharedPtr<FJsonObject> BKHTTPWrapper::EmptyJsonObject = MakeShared<FJsonObject>();

FString BKHTTPWrapper::GetRequestUrl() const
{
	return RequestUrl;
}

FString BKHTTPWrapper::TokenCached = "";

void BKHTTPWrapper::Initialize(
	TWeakPtr<BKHTTPWrapper> WeakPointer,
	const FString& InRequestSummary,
	const FString& Url,
	TFunction<void(TWeakPtr<BKHTTPWrapper>, const FString&, int32)> OnFailure,
	TFunction<void(TWeakPtr<BKHTTPWrapper>, const FString&)> OnSuccess)
{
	WeakSelf = WeakPointer;

	RequestSummary = InRequestSummary;
	if (OnFailure)
	{
		OnHttpRequestFailed.AddLambda(OnFailure);
	}
	if (OnSuccess)
	{
		OnHttpRequestSucceed.AddLambda(OnSuccess);
	}
	RequestUrl = Url;
}

void BKHTTPWrapper::AppendBindings(
	TFunction<void(TWeakPtr<BKHTTPWrapper>, const FString&, int32)> OnFailure,
	TFunction<void(TWeakPtr<BKHTTPWrapper>, const FString&)> OnSuccess)
{
	if (OnFailure)
	{
		OnHttpRequestFailed.AddLambda(OnFailure);
	}
	if (OnSuccess)
	{
		OnHttpRequestSucceed.AddLambda(OnSuccess);
	}
}

void BKHTTPWrapper::AppendCancellationToken(UBKCancellationToken* InCancellationToken)
{
	if (InCancellationToken && InCancellationToken->IsValidLowLevel() && !InCancellationToken->IsPendingKillOrUnreachable())
	{
		AppendCancellationToken(InCancellationToken->GetData());
	}
}
void BKHTTPWrapper::AppendCancellationToken(TSharedPtr<BKCancellationTokenWeakWrapper> InCancellationToken)
{
	if (InCancellationToken.IsValid())
	{
		for (int32 i = 0; i < CancellationTokens.Num(); i++)
		{
			if (CancellationTokens[i].IsValid() && CancellationTokens[i]->Equals(InCancellationToken))
			{
				return;
			}
		}
		bWithCancellationToken = true;
		CancellationTokens.Add(InCancellationToken);
	}
}

void BKHTTPWrapper::Get()
{
	HTTPRequestType = EBKHTTPRequestType::GET;
	bCanTick = true;
	Perform();
}

void BKHTTPWrapper::Delete()
{
	HTTPRequestType = EBKHTTPRequestType::DELETE;
	bCanTick = true;
	Perform();
}

void BKHTTPWrapper::Post(const TSharedPtr<FJsonObject>& Content)
{
	HTTPRequestType = EBKHTTPRequestType::POST;
	TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<>::Create(&RequestContent);
	FJsonSerializer::Serialize(Content.ToSharedRef(), JsonWriter);
	bCanTick = true;
	Perform();
}

void BKHTTPWrapper::Put(const TSharedPtr<FJsonObject>& Content)
{
	HTTPRequestType = EBKHTTPRequestType::PUT;
	TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<>::Create(&RequestContent);
	FJsonSerializer::Serialize(Content.ToSharedRef(), JsonWriter);
	bCanTick = true;
	Perform();
}

void BKHTTPWrapper::Tick(float DeltaTime)
{
	if (bCanceled) return;

	//UE_LOG(LogTemp, Warning, TEXT("Tick: %s"), *this->GetRequestUrl());
	if (RetryTimer >= 0.0f)
	{
		RetryTimer += DeltaTime;
		if (RetryTimer >= 1.0f)
		{
			RetryTimer = -1.0;
			if (NumberOfRetries++ <= MAX_NUMBER_OF_RETRIES)
			{
				Perform();
			}
			else
			{
				bCanTick = false;
				OnHttpRequestFailed.Broadcast(WeakSelf, LastErrorMessage, LastErrorCode);
			}
		}
	}
	else if (bWithCancellationToken)
	{
		bool bFoundActive = false;
		for (int32 i = CancellationTokens.Num() - 1; i >= 0; i--)
		{
			if (CancellationTokens[i].IsValid())
			{
				bFoundActive = (CancellationTokens[i].IsValid() && CancellationTokens[i]->IsTokenValid());
				if (bFoundActive)
				{
					break;
				}
			}
			else
			{
				CancellationTokens.RemoveAt(i);
			}
		}
		if (!bFoundActive)
		{
			auto PinnedRequest = HttpRequestWeakPtr.Pin();
			if (PinnedRequest.IsValid())
			{
				bCanceled = true;
				bCanTick = false;
				UE_LOG(LogTemp, Warning, TEXT("Request has been canceled: %s"), *GetRequestUrl());
				PinnedRequest->CancelRequest();
				OnHttpRequestFailed.Broadcast(WeakSelf, TEXT("Canceled"), 500);
			}
		}
	}
}

bool BKHTTPWrapper::IsTickable() const
{
	return bCanTick;
}

TStatId BKHTTPWrapper::GetStatId() const
{
	return TStatId();
}

bool BKHTTPWrapper::GetResponseObject(TSharedPtr<class FJsonObject>& OutResponse)
{
	if (ResponseJsonObject == nullptr)
		return false;

	OutResponse = ResponseJsonObject;
	return true;
}

void BKHTTPWrapper::Perform()
{
 	if (HTTPRequestType == EBKHTTPRequestType::NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("Request type must be defined by calling Get(), Delete(), Post(), Put() calls."));
		return;
	}

	if (bWithCancellationToken)
	{
		bool bFoundActive = false;
		for (int32 i = CancellationTokens.Num() - 1; i >= 0; i--)
		{
			if (CancellationTokens[i].IsValid())
			{
				bFoundActive = (CancellationTokens[i].IsValid() && CancellationTokens[i]->IsTokenValid());
				if (bFoundActive)
				{
					break;
				}
			}
			else
			{
				CancellationTokens.RemoveAt(i);
			}
		}
		if (!bFoundActive)
		{
			bCanceled = true;
			bCanTick = false;
			OnHttpRequestFailed.Broadcast(WeakSelf, TEXT("Canceled"), 500);
			return;
		}
	}

	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequestWeakPtr = HttpRequest;

	switch (HTTPRequestType)
	{
		case EBKHTTPRequestType::GET:
		{
			HttpRequest->SetVerb("GET");
			break;
		}
		case EBKHTTPRequestType::POST:
		{
			HttpRequest->SetVerb("POST");
			HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
			HttpRequest->SetContentAsString(RequestContent);
			break;
		}
		case EBKHTTPRequestType::PUT:
		{
			HttpRequest->SetVerb("PUT");
			HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
			HttpRequest->SetContentAsString(RequestContent);
			break;
		}
		case EBKHTTPRequestType::DELETE:
		{
			HttpRequest->SetVerb("DELETE");
			break;
		}
	}

	HttpRequest->SetURL(RequestUrl);
	HttpRequest->SetHeader(TEXT("Authorization"), TokenCached);

	HttpRequest->OnProcessRequestComplete().BindRaw(this, &BKHTTPWrapper::HttpResponseReceived);
	HttpRequest->ProcessRequest();
}

void BKHTTPWrapper::HttpResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectionWasSuccessful)
{
	if (bCanceled) return;

	if (!bConnectionWasSuccessful || !Response.IsValid())
	{
		LastErrorCode = 500;
		LastErrorMessage = "Connection has failed. Check network connectivity.";
		RetryTimer = 0.0f; //Start timer for retry
		return;
	}

	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (!FJsonSerializer::Deserialize(Reader, ResponseJsonObject))
	{
		LastErrorCode = 500;
		LastErrorMessage = "Server returned an unexpected response.";
		RetryTimer = 0.0f; //Start timer for retry
		return;
	}

	if (Response->GetResponseCode() >= 500 || Response->GetResponseCode() == EHttpResponseCodes::Forbidden)
	{
		LastErrorCode = Response->GetResponseCode();
		LastErrorMessage = ResponseJsonObject->HasTypedField<EJson::String>("message") ? ResponseJsonObject->GetStringField("message") : Response->GetContentAsString();
		RetryTimer = 0.0f; //Start timer for retry
		if (Response->GetResponseCode() == 503) //Maintenance mode
		{
			NumberOfRetries = 0; //Retry infinitely
		}
		return;
	}

	// Notify user if Http response code is 401 - Login needed
	if (Response->GetResponseCode() == EHttpResponseCodes::Denied)
	{
		LastErrorCode = 401;
		LastErrorMessage = "Login to authenticate";
		bCanTick = false;
		OnHttpRequestFailed.Broadcast(WeakSelf, LastErrorMessage, LastErrorCode);
		return;
	}

	//Other errors
	if (Response->GetResponseCode() >= 400)
	{
		FString RequestBody = FString(ANSI_TO_TCHAR((char*)Request->GetContent().GetData()));

		LastErrorCode = Response->GetResponseCode();
		LastErrorMessage = ResponseJsonObject->HasTypedField<EJson::String>("message") ? ResponseJsonObject->GetStringField("message") : Response->GetContentAsString();
		bCanTick = false;
		OnHttpRequestFailed.Broadcast(WeakSelf, LastErrorMessage, LastErrorCode);
		return;
	}

	OnHttpRequestSucceed.Broadcast(WeakSelf, Response->GetContentAsString());
}
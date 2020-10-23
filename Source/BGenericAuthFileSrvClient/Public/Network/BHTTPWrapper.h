/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "CoreMinimal.h"
#include "TickableEditorObject.h"
#include "UObject/NoExportTypes.h"
#include "Runtime/Online/HTTP/Public/Http.h"

enum EBKHTTPRequestType : int8
{
	NONE = 0,
	GET = 1,
	POST = 2,
	PUT = 3,
	DELETE = 4
};

class BGENERICAUTHFILESRVCLIENT_API BKHTTPWrapper : public FTickableEditorObject
{

public:
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;

	bool GetResponseObject(TSharedPtr<class FJsonObject>& OutResponse);

	DECLARE_MULTICAST_DELEGATE_ThreeParams(FRVHttpRequestFailed, TWeakPtr<BKHTTPWrapper>, const FString&, int32);
	FRVHttpRequestFailed OnHttpRequestFailed;

	DECLARE_MULTICAST_DELEGATE_TwoParams(FRVHttpRequestSucceed, TWeakPtr<BKHTTPWrapper>, const FString&);
	FRVHttpRequestSucceed OnHttpRequestSucceed;

	static const TSharedPtr<class FJsonObject> EmptyJsonObject;

	FString GetRequestUrl() const;

private:
	friend class UBKNetworkManager;

	TWeakPtr<BKHTTPWrapper> WeakSelf;

	void Initialize(
		TWeakPtr<BKHTTPWrapper> WeakPointer,
		const FString& InRequestSummary,
		const FString& Url,
		TFunction<void(TWeakPtr<BKHTTPWrapper>, const FString&, int32)> OnFailure = nullptr,
		TFunction<void(TWeakPtr<BKHTTPWrapper>, const FString&)> OnSuccess = nullptr);
	void AppendBindings(
		TFunction<void(TWeakPtr<BKHTTPWrapper>, const FString&, int32)> OnFailure = nullptr,
		TFunction<void(TWeakPtr<BKHTTPWrapper>, const FString&)> OnSuccess = nullptr);
	void AppendCancellationToken(class UBKCancellationToken* InCancellationToken);
	void AppendCancellationToken(TSharedPtr<class BKCancellationTokenWeakWrapper> InCancellationToken);

	void Get();
	void Delete();
	void Post(const class TSharedPtr<class FJsonObject>& Content);
	void Put(const class TSharedPtr<class FJsonObject>& Content);

	static FString TokenCached;

	void Perform();
	void HttpResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectionWasSuccessful);

	EBKHTTPRequestType HTTPRequestType = EBKHTTPRequestType::NONE;
	TSharedPtr<class FJsonObject> ResponseJsonObject = nullptr;
	FString RequestUrl;
	FString RequestContent = "";
	FString RequestSummary;

	TArray<TSharedPtr<class BKCancellationTokenWeakWrapper>> CancellationTokens;
	bool bWithCancellationToken = false;
	TWeakPtr<IHttpRequest> HttpRequestWeakPtr;

	const int32 MAX_NUMBER_OF_RETRIES = 10;
	bool bCanTick = false;
	int32 NumberOfRetries = 0;
	float RetryTimer = -1.0f;
	int32 LastErrorCode;
	FString LastErrorMessage;

	bool bCanceled = false;
};
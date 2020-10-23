/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "Managers/BManager.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "BNetworkManager.generated.h"

namespace BFetchApiEndpoint
{
    class BProcess
    {
    private:
        static FString CachedBApiEndpointBase;
        static FString BApiEndpointBaseIniPath;
    public:
        static FString GetBApiEndpointBase();
    };
}
#define BASE_PATH BFetchApiEndpoint::BProcess::GetBApiEndpointBase()

#define END_POINT_AUTH_USER "auth/users/"
#define END_POINT_FILE_MODELS "file/models/"

#define SAVE_FILE_PATH FPaths::ProjectPluginsDir() + "BGenericAuthFileSrvClient/Saved/LastNetworkUser.json"

enum EBKHTTPRequestType : int8;

USTRUCT()
struct BGENERICAUTHFILESRVCLIENT_API FNetworkUser
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY()
    FString Id;
    UPROPERTY()
    FString Token;

    // Not stored
    UPROPERTY()
    FString UserName;
    UPROPERTY()
    FString Email;

    FNetworkUser() {}
};

UCLASS()
class BGENERICAUTHFILESRVCLIENT_API UBKNetworkManager : public UBKManager
{
	GENERATED_BODY()

protected:
	virtual void OnConstruct() override;
	virtual void OnDestruct() override;

public:
	bool StartLoginProcedure(bool bUseCancellationToken, TSharedPtr<class BKCancellationTokenWeakWrapper> CancellationToken);
    void StopLoginProcedure();
	void Logout();

    DECLARE_MULTICAST_DELEGATE_OneParam(FRVLoginFailed, const FString&);
    FRVLoginFailed OnLoginFailed;

    DECLARE_MULTICAST_DELEGATE_OneParam(FRVLoginSucceed, const FString&);
    FRVLoginSucceed OnLoginSucceed;

    void BroadcastLoginStatus(bool bLoggedIn);

    void StoreNetworkUser(const FString& UserId, const FString& UserToken);

    TWeakPtr<class BKHTTPWrapper> Get(const FString& Url, TFunction<void(TWeakPtr<class BKHTTPWrapper>, const FString&, int32)> OnFailure, TFunction<void(TWeakPtr<class BKHTTPWrapper>, const FString&)> OnSuccess, bool bUseCancellationToken = false, TSharedPtr<class BKCancellationTokenWeakWrapper> WithCancellationToken = nullptr);

    TWeakPtr<class BKHTTPWrapper> Delete(const FString& Url, TFunction<void(TWeakPtr<class BKHTTPWrapper>, const FString&, int32)> OnFailure, TFunction<void(TWeakPtr<class BKHTTPWrapper>, const FString&)> OnSuccess, bool bUseCancellationToken = false, TSharedPtr<class BKCancellationTokenWeakWrapper> WithCancellationToken = nullptr);

    TWeakPtr<class BKHTTPWrapper> Post(const FString& Url, const FString& StringContent, TFunction<void(TWeakPtr<class BKHTTPWrapper>, const FString&, int32)> OnFailure, TFunction<void(TWeakPtr<class BKHTTPWrapper>, const FString&)> OnSuccess, bool bUseCancellationToken = false, TSharedPtr<class BKCancellationTokenWeakWrapper> WithCancellationToken = nullptr);
    TWeakPtr<class BKHTTPWrapper> Post(const FString& Url, const TSharedPtr<class FJsonObject>& JsonContent, TFunction<void(TWeakPtr<class BKHTTPWrapper>, const FString&, int32)> OnFailure, TFunction<void(TWeakPtr<class BKHTTPWrapper>, const FString&)> OnSuccess, bool bUseCancellationToken = false, TSharedPtr<class BKCancellationTokenWeakWrapper> WithCancellationToken = nullptr);

    TWeakPtr<class BKHTTPWrapper> Put(const FString& Url, const FString& StringContent, TFunction<void(TWeakPtr<class BKHTTPWrapper>, const FString&, int32)> OnFailure, TFunction<void(TWeakPtr<class BKHTTPWrapper>, const FString&)> OnSuccess, bool bUseCancellationToken = false, TSharedPtr<class BKCancellationTokenWeakWrapper> WithCancellationToken = nullptr);
    TWeakPtr<class BKHTTPWrapper> Put(const FString& Url, const TSharedPtr<class FJsonObject>& JsonContent, TFunction<void(TWeakPtr<class BKHTTPWrapper>, const FString&, int32)> OnFailure, TFunction<void(TWeakPtr<class BKHTTPWrapper>, const FString&)> OnSuccess, bool bUseCancellationToken = false, TSharedPtr<class BKCancellationTokenWeakWrapper> WithCancellationToken = nullptr);

    bool ValidateUser(bool bUseCancellationToken, TSharedPtr<class BKCancellationTokenWeakWrapper> CancellationToken);

	UFUNCTION(BlueprintCallable, Category = "BGenericAuthFileSrvClient|Managers|Network Manager")
	void DeleteCurrentNetworkUserData();

    UFUNCTION(BlueprintCallable, Category = "BGenericAuthFileSrvClient|Managers|Network Manager")
    bool GetCurrentUserId(FString& UserId);
	
	UFUNCTION(BlueprintCallable, Category = "BGenericAuthFileSrvClient|Managers|Network Manager")
    bool GetCurrentUserToken(FString& Token);

	UFUNCTION(BlueprintCallable, Category = "BGenericAuthFileSrvClient|Managers|Network Manager")
    bool GetCurrentUserName(FString& UserName);

	UFUNCTION(BlueprintCallable, Category = "BGenericAuthFileSrvClient|Managers|Network Manager")
	bool GetCurrentUserEmail(FString& UserEmail);

    UFUNCTION(BlueprintCallable, Category = "BGenericAuthFileSrvClient|Managers|Network Manager")
    void RegisterAuthenticatedWidgetObserver(UBKViewWidget* Observer);

	UFUNCTION(BlueprintCallable, Category = "BGenericAuthFileSrvClient|Managers|Network Manager")
	void UnregisterAuthenticatedWidgetObserver(UBKViewWidget* Observer);

	void UnregisterAllAuthenticatedWidgetObservers();
	TArray<TWeakObjectPtr<class UBKViewWidget>> GetObserverWidgetsCopy() const;

	virtual void Tick(float DeltaTime) override;

    TSharedPtr<class BHttpServer> Server;
    TSharedPtr<class BKLoginBrowserListener> Listener;

private:
	virtual void OnMapIsBeingTornDown() override;

	bool StartLoginProcedure_Internal();

    bool LoadNetworkUser();
    void FlagHttpWrapperForDeletion(TWeakPtr<class BKHTTPWrapper> HttpWrapper);

    void NotifyAuthenticatedWidgetObservers(bool bAuthenticated);

	//Step 1
	FString CalculateRequestSummary(
        const FString& Url,
		EBKHTTPRequestType RequestType,
		const TSharedPtr<class FJsonObject>& JsonContent,
		bool bUseCancellationToken = false);
    FString CalculateRequestSummary(
        const FString& Url, 
        EBKHTTPRequestType RequestType,
        const FString& Stringified, 
        bool bUseCancellationToken);

    //Step 2
    TWeakPtr<class BKHTTPWrapper> CheckForAnIdenticalRequestAndBindIfExists(
		const FString& RequestSummary,
		TFunction<void(TWeakPtr<class BKHTTPWrapper>, const FString&, int32)> OnFailure,
		TFunction<void(TWeakPtr<class BKHTTPWrapper>, const FString&)> OnSuccess,
		bool bUseCancellationToken,
        TSharedPtr<class BKCancellationTokenWeakWrapper> WithCancellationToken);

	//Step 3
    TWeakPtr<class BKHTTPWrapper> NewNonExistentHttpWrapperPrechecked(
		const FString& RequestSummary,
		const FString& Url,
		TFunction<void(TWeakPtr<class BKHTTPWrapper>, const FString&, int32)> OnFailure,
		TFunction<void(TWeakPtr<class BKHTTPWrapper>, const FString&)> OnSuccess,
		bool bUseCancellationToken = false,
        TSharedPtr<class BKCancellationTokenWeakWrapper> CancellationToken = nullptr);

	TArray<TSharedPtr<class BKHTTPWrapper>> HttpWrappers;
    TMap<FString, TWeakPtr<class BKHTTPWrapper>> RequestSummaryHttpWrapperMap;

    UPROPERTY()
    FNetworkUser CurrentNetworkUser;

    TArray<TWeakObjectPtr<class UBKViewWidget>> ObserverWidgets;

    bool bCanTick = false;
};
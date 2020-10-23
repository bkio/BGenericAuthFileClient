/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "Actions/BActionBlueprintFunctionLibrary.h"
#include "Managers/BManagerStore.h"
#include "Managers/BActionManager.h"
#include "Managers/BNetworkManager.h"
#include "Network/BHTTPWrapper.h"
#include "Engine/LatentActionManager.h"
#include "JsonUtilities.h"
#include "Misc/DefaultValueHelper.h"
#include "Components/Widget.h"
#include "Network/BCancellationToken.h"
#include "Actions/BActionBase.h"

void UBKActionBlueprintFunctionLibrary::Execute_DownloadFile_Action(
	UBKActionBase* Action,
	const FString& UrlPostFix,
	const FString& DestinationPath,
	const FString& ModelId,
	int32 RevisionIndex,
	int32 VersionIndex,
	const TFunction<void(float)>& ProgressCallback,
	const TFunction<void(const FString&, int32)>& FailureCallback,
	const TFunction<void(const FString&)>& SuccessCallback)
{
	TWeakObjectPtr<UBKActionBase> ActionPtr(Action);

	FString Url = BASE_PATH;
	Url.Append(END_POINT_FILE_MODELS + ModelId + "/revisions/" + FString::FromInt(RevisionIndex) + "/versions/" + FString::FromInt(VersionIndex) + "/" + UrlPostFix);

	//First call is for getting file download url from google cloud storage (signed url)
	UBKManagerStore::GetNetworkManager()->Get(Url,
		[ActionPtr, FailureCallback]
		(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message, int32 ErrorCode)
		{
			if (!ActionPtr.IsValid()) return;
			if (ActionPtr->IsActionCanceled()) return;

			FailureCallback(Message, ErrorCode);
		},
		[ActionPtr, ProgressCallback, FailureCallback, SuccessCallback, DestinationPath]
		(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message)
		{
			if (!ActionPtr.IsValid()) return;
			if (ActionPtr->IsActionCanceled()) return;
			if (!HttpWrapper.IsValid()) return;

			TSharedPtr<FJsonObject> JsonObj;
			HttpWrapper.Pin()->GetResponseObject(JsonObj);
			FString DownloadUrl = JsonObj->GetStringField(FILE_DOWNLOAD_URL_PROPERTY);

			//Second call is for sending a HEAD request to the gcs url to get total file size

			TSharedRef<IHttpRequest> SecondHttpRequest = FHttpModule::Get().CreateRequest();
			SecondHttpRequest->SetVerb("HEAD");
			SecondHttpRequest->SetURL(DownloadUrl);
			SecondHttpRequest->OnProcessRequestComplete().BindLambda([ActionPtr, DownloadUrl, DestinationPath, ProgressCallback, FailureCallback, SuccessCallback]
				(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
				{
					if (!ActionPtr.IsValid()) return;
					if (ActionPtr->IsActionCanceled()) return;

					if (bConnectedSuccessfully 
						&& Response->GetResponseCode() < 400)
					{
						FString LengthHeader = Response->GetHeader("content-length");
						int64 Length;
						if (FDefaultValueHelper::ParseInt64(LengthHeader, Length))
						{
							TSharedRef<IHttpRequest> ThirdHttpRequest = FHttpModule::Get().CreateRequest();
							ThirdHttpRequest->SetVerb("GET");
							ThirdHttpRequest->SetURL(DownloadUrl);
							ThirdHttpRequest->OnRequestProgress().BindLambda([ActionPtr, ProgressCallback, Length]
								(FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived)
								{
									if (!ActionPtr.IsValid()) return;

									if (ActionPtr->IsActionCanceled())
									{
										if (Request.IsValid())
										{
											Request->CancelRequest();
										}
										return;
									}

									if (ProgressCallback)
									{
										uint32 Casted = (uint32)BytesReceived;
										ProgressCallback((float)(((double)Casted) / ((double)Length)));
									}
								});
							ThirdHttpRequest->OnProcessRequestComplete().BindLambda([ActionPtr, FailureCallback, SuccessCallback, DestinationPath]
								(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
								{
									if (!ActionPtr.IsValid()) return;
									if (ActionPtr->IsActionCanceled()) return;

									if (!bConnectedSuccessfully || !Response.IsValid())
									{
										if (FailureCallback)
										{
											FailureCallback("Failed to download the file.", 500);
										}
										return;
									}

									int32 ResponseCode = Response->GetResponseCode();
									if (ResponseCode < 400)
									{
										if (!FFileHelper::SaveArrayToFile(Response->GetContent(), *DestinationPath)
											|| !FPaths::FileExists(DestinationPath))
										{
											if (FailureCallback)
											{
												FailureCallback("Failed to save the file.", 400);
											}
										}
										else if (SuccessCallback)
										{
											SuccessCallback("Download successful.");
										}
									}
									else if (FailureCallback)
									{
										FailureCallback("Failed to download the file.", ResponseCode);
									}
								});
							ThirdHttpRequest->ProcessRequest();
						}
						else
						{
							if (FailureCallback)
							{
								FailureCallback("Http request has failed; content-length response header is invalid: " + LengthHeader, 500);
							}
						}
					}
					else if (FailureCallback)
					{
						if (Response.IsValid())
						{
							FailureCallback("Http request has failed with: " + Response->GetContentAsString(), 500);
						}
						else
						{
							FailureCallback("Http request has failed.", 500);
						}
					}
				});
			SecondHttpRequest->ProcessRequest();
		},
		ActionPtr->InputPtr ? ActionPtr->InputPtr->bUseCancellationToken : false,
		(ActionPtr->InputPtr && ActionPtr->InputPtr->CancellationToken && ActionPtr->InputPtr->CancellationToken->IsValidLowLevel() && !ActionPtr->InputPtr->CancellationToken->IsPendingKillOrUnreachable()) ? ActionPtr->InputPtr->CancellationToken->GetData() : nullptr);
}

FString UBKActionBlueprintFunctionLibrary::MakeFileId(const FString& ModelId, const int32& RevisionIndex, const int32& VersionIndex)
{
	FString FileId;
	FileId.Append(ModelId);
	FileId.Append(":");
	FileId.Append(FString::FromInt(RevisionIndex));
	FileId.Append(":");
	FileId.Append(FString::FromInt(VersionIndex));
	return FileId;
}
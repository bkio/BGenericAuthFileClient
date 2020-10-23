/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "Actions/BActionUploadRawFile.h"
#include "Managers/BManagerStore.h"
#include "Managers/BActionManager.h"
#include "Managers/BNetworkManager.h"
#include "Network/BHTTPWrapper.h"
#include "Engine/LatentActionManager.h"
#include "JsonUtilities.h"
#include "BZipFile.h"
#include "BLambdaRunnable.h"
#include "Network/BCancellationToken.h"

#define SAVE_FOLDER_PATH FPaths::ProjectPluginsDir() + "BGenericAuthFileSrvClient/Saved/"

void UBKActionUploadRawFile::Execute()
{
	Super::Execute();

	/*
	* First http call part
	*/

	FString Url = BASE_PATH;
	Url.Append(END_POINT_FILE_MODELS + Input.ModelId + "/revisions/" + FString::FromInt(Input.RevisionIndex) + "/versions/" + FString::FromInt(Input.VersionIndex) + "/raw");

	TWeakObjectPtr<UBKActionUploadRawFile> ThisPtr(this);
	UBKManagerStore::GetNetworkManager()->Post(Url, JsonString,
		[ThisPtr]
	(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message, int32 ErrorCode)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled())
			{
				ThisPtr->OnUploadRawFileFailed.Broadcast(Message, ErrorCode);
				
				return;
			}
		},
		[ThisPtr]
		(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled() && HttpWrapper.IsValid())
			{
				TSharedPtr<FJsonObject> JsonObj;
				HttpWrapper.Pin()->GetResponseObject(JsonObj);
				FString UploadUrl = JsonObj->GetStringField(FILE_UPLOAD_URL_PROPERTY);
				FString UploadContentType = JsonObj->GetStringField(FILE_UPLOAD_CONTENT_TYPE_PROPERTY);
				FBLambdaRunnable::RunLambdaOnDedicatedBackgroundThread([ThisPtr, UploadUrl, UploadContentType]()
					{
						if (!ThisPtr.IsValid() || ThisPtr->IsActionCanceled()) return;

						/*
						* Zip creation part
						*/

						FString GeneratedZipName = UBKActionManager::GenerateRandomANSIString().Mid(0, 16) + ".zip";
						FString ZipPath = FPaths::ConvertRelativePathToFull(SAVE_FOLDER_PATH + GeneratedZipName);

						FString ErrorMessage;
						TSharedPtr<BZipArchive> Archive;
						if (!BZipFile::Open(Archive, ZipPath, ErrorMessage))
						{
							FBLambdaRunnable::RunLambdaOnGameThread([ErrorMessage, ThisPtr]()
								{
									if (!ThisPtr.IsValid() || ThisPtr->IsActionCanceled()) return;

									ThisPtr->OnUploadRawFileFailed.Broadcast(ErrorMessage, 500);
								});
							return;
						}
						if (!ThisPtr->CreateEntriesAndOpenFileStreams(Archive))
						{
							ThisPtr->CloseOpenedFileStreams();
							FBLambdaRunnable::RunLambdaOnGameThread([ThisPtr]()
								{
									if (!ThisPtr.IsValid() || ThisPtr->IsActionCanceled()) return;
									ThisPtr->OnUploadRawFileFailed.Broadcast(TEXT("Cannot open files, please check application permissions."), 500);
								});
							return;
						}
						if (!BZipFile::SaveAndClose(Archive, ZipPath, ErrorMessage))
						{
							ThisPtr->CloseOpenedFileStreams();
							FBLambdaRunnable::RunLambdaOnGameThread([ErrorMessage, ThisPtr]()
								{
									if (!ThisPtr.IsValid() || ThisPtr->IsActionCanceled()) return;

									ThisPtr->OnUploadRawFileFailed.Broadcast(ErrorMessage, 500);
								});
							return;
						}
						ThisPtr->CloseOpenedFileStreams();
						int64 ZipFileSize = IFileManager::Get().FileSize(*ZipPath);
						if (ZipFileSize <= 0)
						{
							FBLambdaRunnable::RunLambdaOnGameThread([ThisPtr]()
								{
									if (!ThisPtr.IsValid() || ThisPtr->IsActionCanceled()) return;

									ThisPtr->OnUploadRawFileFailed.Broadcast(TEXT("Cannot access to created zip file, please check application permissions."), 500);
								});
							return;
						}
						/*
						* Upload request (second request) part
						*/
						FBLambdaRunnable::RunLambdaOnGameThread([ThisPtr, ZipPath, UploadUrl, UploadContentType, ZipFileSize]()
							{
								if (!ThisPtr.IsValid()) return;

								//If canceled
								if (ThisPtr->IsActionCanceled())
								{
									//Delete the zip file
									FBLambdaRunnable::RunLambdaOnDedicatedBackgroundThread([ZipPath]()
										{
											IFileManager::Get().Delete(*ZipPath);
										});
									return;
								}

								TSharedRef<IHttpRequest> SecondHttpRequest = FHttpModule::Get().CreateRequest();
								SecondHttpRequest->SetVerb("PUT");
								SecondHttpRequest->SetURL(UploadUrl);
								SecondHttpRequest->SetHeader("Content-Type", UploadContentType);
								SecondHttpRequest->SetHeader("Content-Length", UBKActionBase::ConvertInt64ToFString(ZipFileSize));
								SecondHttpRequest->OnRequestProgress().BindLambda(
									[ThisPtr, ZipPath, ZipFileSize]
								(FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived)
									{
										if (!ThisPtr.IsValid()) return;

										if (ThisPtr->IsActionCanceled())
										{
											if (Request.IsValid())
											{
												Request->CancelRequest();
											}
											//Delete the zip file
											FBLambdaRunnable::RunLambdaOnDedicatedBackgroundThread([ZipPath]()
												{
													IFileManager::Get().Delete(*ZipPath);
												});
											return;
										}

										uint32 Casted = (uint32)BytesSent;
										ThisPtr->OnUploadRawFileProgress.Broadcast((float)(((double)Casted) / ((double)ZipFileSize)));
									});
								SecondHttpRequest->OnProcessRequestComplete().BindLambda(
									[ThisPtr, ZipPath]
								(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
									{
										//Delete the zip file
										FBLambdaRunnable::RunLambdaOnDedicatedBackgroundThread([ZipPath]()
											{
												IFileManager::Get().Delete(*ZipPath);
											});

										if (!ThisPtr.IsValid() || ThisPtr->IsActionCanceled()) return;

										if (!bConnectedSuccessfully || !Response.IsValid())
										{
											ThisPtr->OnUploadRawFileFailed.Broadcast("Failed to download the file.", 500);
											return;
										}

										int32 ResponseCode = Response->GetResponseCode();
										if (ResponseCode < 400)
										{
											ThisPtr->OnUploadRawFileSucceed.Broadcast("File has been successfully uploaded.");
										}
										else
										{
											ThisPtr->OnUploadRawFileFailed.Broadcast("Failed to download the file.", ResponseCode);
										}
									});
								if (!SecondHttpRequest->SetContentAsStreamedFile(ZipPath))
								{
									ThisPtr->OnUploadRawFileFailed.Broadcast(TEXT("Cannot access to created zip file."), 500);
									return;
								}
								SecondHttpRequest->ProcessRequest();
							});
					});
			}
		},
			Input.bUseCancellationToken,
			(Input.CancellationToken && Input.CancellationToken->IsValidLowLevel() && !Input.CancellationToken->IsPendingKillOrUnreachable()) ? Input.CancellationToken->GetData() : nullptr);
}

void UBKActionUploadRawFile::Perform_UploadRawFile_Action_BP(
	const FBKActionUploadRawFileInput& Input,
	BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
	FBKActionUploadRawFileOutput& Output,
	struct FLatentActionInfo LatentInfo)
{
	UBKActionUploadRawFile* Action = NewObject<UBKActionUploadRawFile>(UBKManagerStore::GetActionManager());

	FBKActionUploadRawFileInput NewInput;
	NewInput.ActionId = Input.ActionId;
	NewInput.bUseCancellationToken = Input.bUseCancellationToken;
	NewInput.CancellationToken = Input.CancellationToken;
	NewInput.FileEntryName = Input.FileEntryName;
	NewInput.FilePaths = Input.FilePaths;
	NewInput.ModelId = Input.ModelId;
	NewInput.PotentialCADSoftwareNames = Input.PotentialCADSoftwareNames;
	NewInput.RevisionIndex = Input.RevisionIndex;
	NewInput.VersionIndex = Input.VersionIndex;

	Action->Input = NewInput;
	Action->Output.RelativeInput = NewInput;
	Action->Output.InputPtr = &Action->Input;
	Action->SetInputOutputPointers((FBKInputBase*)&Action->Input, (FBKOutputBase*)&Action->Output);

	FVersionFileEntry FileEntry = MakeVersionFileEntryForUpload(Input);

	Action->InitializeActionDefinition(FVersionFileEntry::StaticStruct(), FileEntry)
		->ThisIsAnUpsertActionCustom(&FileEntry,
			[Action](FBKDataStruct* InData)
			{
				((FVersionFileEntry*)InData)->PruneForUploadCall(Action->JsonObj);
			})
		->CalledFromBP(Exec, LatentInfo, &Action->Output, &Output)
				->FinalizeActionDefinition();

			Action->OnUploadRawFileProgress.AddWeakLambda(Action, [Action]
			(float Progress)
				{
					Action->Output.Progress = Progress;
				});
			Action->OnUploadRawFileFailed.AddWeakLambda(Action, [Action]
			(const FString& Message, int32 ErrorCode)
				{
					if (Action->Output.Exec)
						*Action->Output.Exec = BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL::Failed;

					Action->Output.Message = Message;
					Action->Output.ErrorCode = ConvertHttpCodeToActionErrorCode(ErrorCode);

					if (Action->Output.Condition.IsValid())
						*Action->Output.Condition.Get() = true;

					Action->ActionCompleted();
				});
			Action->OnUploadRawFileSucceed.AddWeakLambda(Action, [Action]
			(const FString& Message)
				{
					if (Action->Output.Exec)
						*Action->Output.Exec = BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL::Succeed;

					Action->Output.Message = Message;

					if (Action->Output.Condition.IsValid())
						*Action->Output.Condition.Get() = true;

					Action->ActionCompleted();
				});

			UBKManagerStore::GetActionManager()->QueueAction(Action);
}

void UBKActionUploadRawFile::Perform_UploadRawFile_Action_Cpp(
	const FBKActionUploadRawFileInput& Input,
	const TFunction<void(const FString&, int32)>& FailureCallback,
	const TFunction<void(float)>& OnProgressCallback,
	const TFunction<void(const FString&)>& SuccessCallback)
{
	UBKActionUploadRawFile* Action = NewObject<UBKActionUploadRawFile>(UBKManagerStore::GetActionManager());
	Action->Input = Input;
	Action->SetInputOutputPointers((FBKInputBase*)&Input, nullptr);
	FVersionFileEntry FileEntry = MakeVersionFileEntryForUpload(Input);

	Action->InitializeActionDefinition(FVersionFileEntry::StaticStruct(), FileEntry)
		->ThisIsAnUpsertActionCustom(&FileEntry,
			[Action](FBKDataStruct* InData)
			{
				((FVersionFileEntry*)InData)->PruneForUploadCall(Action->JsonObj);
			})
		->FinalizeActionDefinition();

			Action->OnUploadRawFileProgress.AddWeakLambda(Action, [Action, OnProgressCallback]
			(float Progress)
				{
					OnProgressCallback(Progress);
				});
			Action->OnUploadRawFileFailed.AddWeakLambda(Action, [Action, FailureCallback]
			(const FString& Message, int32 ErrorCode)
				{
					FailureCallback(Message, ErrorCode);
					Action->ActionCompleted();
				});
			Action->OnUploadRawFileSucceed.AddWeakLambda(Action, [Action, SuccessCallback]
			(const FString& Message)
				{
					SuccessCallback(Message);
					Action->ActionCompleted();
				});

			UBKManagerStore::GetActionManager()->QueueAction(Action);
}

FVersionFileEntry UBKActionUploadRawFile::MakeVersionFileEntryForUpload(const FBKActionUploadRawFileInput& Input)
{
	FVersionFileEntry FileEntry;

	FileEntry.fileEntryName = Input.FileEntryName;
	FileEntry.fileEntryFileType = TEXT("zip");
	FileEntry.zipMainAssemblyFileNameIfAny = Input.TopAssemblyNameIfAny;
	FileEntry.dataSource = "";
	for (auto& Source : Input.PotentialCADSoftwareNames)
	{
		FileEntry.dataSource = Source + "&";
	}
	FileEntry.dataSource.RemoveFromEnd("&");

	return FileEntry;
}

bool UBKActionUploadRawFile::CreateEntriesAndOpenFileStreams(TSharedPtr<BZipArchive> Archive)
{
	auto Method = DeflateMethod::Create();
	Method->SetCompressionLevel(DeflateMethod::CompressionLevel::Fastest);

	for (auto& CurrentFilePath : Input.FilePaths)
	{
		auto Entry = Archive->CreateEntry(FPaths::GetCleanFilename(CurrentFilePath));

		TSharedPtr<std::ifstream> ContentStream = MakeShareable(new std::ifstream());
		ContentStream->open(TCHAR_TO_UTF8(*CurrentFilePath), std::ios::binary);

		if (!ContentStream->is_open())
		{
			return false;
		}

		OpenedFileStreams.Add(ContentStream);

		if (!Entry->SetCompressionStream(*ContentStream.Get(), Method, BZipArchiveEntry::CompressionMode::Deferred))
		{
			return false;
		}
	}
	return true;
}

void UBKActionUploadRawFile::CloseOpenedFileStreams()
{
	for (auto& CurrentStream : OpenedFileStreams)
	{
		if (CurrentStream.IsValid() && CurrentStream->is_open())
		{
			CurrentStream->close();
		}
	}
	OpenedFileStreams.Empty();
}
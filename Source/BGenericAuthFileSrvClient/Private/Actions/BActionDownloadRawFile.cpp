/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "Actions/BActionDownloadRawFile.h"
#include "Managers/BManagerStore.h"
#include "Managers/BActionManager.h"
#include "Managers/BNetworkManager.h"
#include "Network/BHTTPWrapper.h"
#include "Engine/LatentActionManager.h"
#include "JsonUtilities.h"
#include "Actions/BActionBlueprintFunctionLibrary.h"

void UBKActionDownloadRawFile::Execute()
{
	Super::Execute();

	TWeakObjectPtr<UBKActionDownloadRawFile> ThisPtr(this);

	UBKActionBlueprintFunctionLibrary::Execute_DownloadFile_Action(
		this,
		FString(TEXT("raw")),
		Input.DestinationLocalPath,
		Input.ModelId,
		Input.RevisionIndex,
		Input.VersionIndex,
		[ThisPtr](float Progress)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled())
			{
				ThisPtr->OnDownloadRawFileProgress.Broadcast(Progress);
			}
		},
		[ThisPtr](const FString& Message, int32 ErrorCode)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled())
			{
				ThisPtr->OnDownloadRawFileFailed.Broadcast(Message, ErrorCode);
			}
		},
		[ThisPtr](const FString& Message)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled())
			{
				ThisPtr->OnDownloadRawFileSucceed.Broadcast(Message);
			}
		});
}

void UBKActionDownloadRawFile::Perform_DownloadRawFile_Action_BP(
	const FBKActionDownloadRawFileInput& Input,
	BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
	FBKActionDownloadRawFileOutput& Output,
	struct FLatentActionInfo LatentInfo)
{
	UBKActionDownloadRawFile* Action = NewObject<UBKActionDownloadRawFile>();

	FBKActionDownloadRawFileInput NewInput;
	NewInput.ActionId = Input.ActionId;
	NewInput.bUseCancellationToken = Input.bUseCancellationToken;
	NewInput.CancellationToken = Input.CancellationToken;
	NewInput.RevisionIndex = Input.RevisionIndex;
	NewInput.VersionIndex = Input.VersionIndex;
	NewInput.DestinationLocalPath = Input.DestinationLocalPath;
	NewInput.ModelId = Input.ModelId;

	Action->Input = NewInput;
	Action->Output.RelativeInput = NewInput;
	Action->Output.InputPtr = &Action->Input;
	Action->SetInputOutputPointers((FBKInputBase*)&Action->Input, (FBKOutputBase*)&Action->Output);

	Action->CalledFromBP(Exec, LatentInfo, &Action->Output, &Output);

	Action->OnDownloadRawFileProgress.AddWeakLambda(Action, [Action]
		(float Progress)
		{
			Action->Output.Progress = Progress;
		});
	Action->OnDownloadRawFileFailed.AddWeakLambda(Action, [Action]
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
	Action->OnDownloadRawFileSucceed.AddWeakLambda(Action, [Action]
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

void UBKActionDownloadRawFile::Perform_DownloadRawFile_Action_Cpp(
	const FBKActionDownloadRawFileInput& Input,
	const TFunction<void(const FString&, int32)>& FailureCallback,
	const TFunction<void(float)>& OnProgressCallback,
	const TFunction<void(const FString&)>& SuccessCallback)
{
	UBKActionDownloadRawFile* Action = NewObject<UBKActionDownloadRawFile>();
	Action->Input = Input;
	Action->SetInputOutputPointers((FBKInputBase*)&Input, nullptr);

	Action->OnDownloadRawFileProgress.AddWeakLambda(Action, [OnProgressCallback]
		(float Progress)
		{
			OnProgressCallback(Progress);
		});
	Action->OnDownloadRawFileFailed.AddWeakLambda(Action, [Action, FailureCallback]
		(const FString& Message, int32 ErrorCode)
		{
			FailureCallback(Message, ErrorCode);
			Action->ActionCompleted();
		});
	Action->OnDownloadRawFileSucceed.AddWeakLambda(Action, [Action, SuccessCallback]
		(const FString& Message)
		{
			SuccessCallback(Message);
			Action->ActionCompleted();
		});

	UBKManagerStore::GetActionManager()->QueueAction(Action);
}
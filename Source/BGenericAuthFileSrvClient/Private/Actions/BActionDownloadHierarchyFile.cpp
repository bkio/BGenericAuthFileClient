/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "Actions/BActionDownloadHierarchyFile.h"
#include "Managers/BManagerStore.h"
#include "Managers/BActionManager.h"
#include "Managers/BNetworkManager.h"
#include "Network/BHTTPWrapper.h"
#include "Engine/LatentActionManager.h"
#include "JsonUtilities.h"
#include "Actions/BActionBlueprintFunctionLibrary.h"

void UBKActionDownloadHierarchyFile::Execute()
{
	Super::Execute();

	TWeakObjectPtr<UBKActionDownloadHierarchyFile> ThisPtr(this);
	UBKActionBlueprintFunctionLibrary::Execute_DownloadFile_Action(
		this,
		FString(TEXT("hierarchy")),
		Input.DestinationLocalPath,
		Input.ModelId,
		Input.RevisionIndex,
		Input.VersionIndex,
		[ThisPtr](float Progress)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled())
			{
				ThisPtr->OnDownloadHierarchyFileProgress.Broadcast(Progress);
			}
		},
		[ThisPtr](const FString& Message, int32 ErrorCode)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled())
			{
				ThisPtr->OnDownloadHierarchyFileFailed.Broadcast(Message, ErrorCode);
			}
		},
		[ThisPtr](const FString& Message)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled())
			{
				ThisPtr->OnDownloadHierarchyFileSucceed.Broadcast(Message);
			}
		});
}

void UBKActionDownloadHierarchyFile::Perform_DownloadHierarchyFile_Action_BP(
	const FBKActionDownloadHierarchyFileInput& Input,
	BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
	FBKActionDownloadHierarchyFileOutput& Output,
	struct FLatentActionInfo LatentInfo)
{
	UBKActionDownloadHierarchyFile* Action = NewObject<UBKActionDownloadHierarchyFile>();
	Action->Input = Input;
	Action->Output.RelativeInput = Action->Input;
	Action->Output.InputPtr = &Action->Input;
	Action->SetInputOutputPointers((FBKInputBase*)&Input, (FBKOutputBase*)&Output);

	Action->CalledFromBP(Exec, LatentInfo, &Action->Output, &Output);

	Action->OnDownloadHierarchyFileProgress.AddWeakLambda(Action, [Action]
		(float Progress)
		{
			Action->Output.Progress = Progress;
		});
	Action->OnDownloadHierarchyFileFailed.AddWeakLambda(Action, [Action]
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
	Action->OnDownloadHierarchyFileSucceed.AddWeakLambda(Action, [Action]
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

void UBKActionDownloadHierarchyFile::Perform_DownloadHierarchyFile_Action_Cpp(
	const FBKActionDownloadHierarchyFileInput& Input,
	const TFunction<void(const FString&, int32)>& FailureCallback,
	const TFunction<void(float)>& OnProgressCallback,
	const TFunction<void(const FString&)>& SuccessCallback)
{
	UBKActionDownloadHierarchyFile* Action = NewObject<UBKActionDownloadHierarchyFile>();
	Action->Input = Input;
	Action->SetInputOutputPointers((FBKInputBase*)&Input, nullptr);

	Action->OnDownloadHierarchyFileProgress.AddWeakLambda(Action, [OnProgressCallback]
		(float Progress)
		{
			OnProgressCallback(Progress);
		});
	Action->OnDownloadHierarchyFileFailed.AddWeakLambda(Action, [Action, FailureCallback]
		(const FString& Message, int32 ErrorCode)
		{
			FailureCallback(Message, ErrorCode);
			Action->ActionCompleted();
		});
	Action->OnDownloadHierarchyFileSucceed.AddWeakLambda(Action, [Action, SuccessCallback]
		(const FString& Message)
		{
			SuccessCallback(Message);
			Action->ActionCompleted();
		});

	UBKManagerStore::GetActionManager()->QueueAction(Action);
}
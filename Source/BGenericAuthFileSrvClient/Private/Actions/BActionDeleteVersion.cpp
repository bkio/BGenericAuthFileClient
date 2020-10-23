/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "Actions/BActionDeleteVersion.h"
#include "Managers/BManagerStore.h"
#include "Managers/BActionManager.h"
#include "Managers/BNetworkManager.h"
#include "Network/BHTTPWrapper.h"
#include "Network/BCancellationToken.h"
#include "Engine/LatentActionManager.h"
#include "JsonUtilities.h"

void UBKActionDeleteVersion::Execute()
{
	Super::Execute();

	FString Url = BASE_PATH;
	Url.Append(END_POINT_FILE_MODELS + Input.ModelId + "/revisions/" + FString::FromInt(Input.RevisionIndex) + "/versions/" + FString::FromInt(Input.VersionIndex));

	TWeakObjectPtr<UBKActionDeleteVersion> ThisPtr(this);

	UBKManagerStore::GetNetworkManager()->Delete(Url,
		[ThisPtr]
		(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message, int32 ErrorCode)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled())
			{
				ThisPtr->OnDeleteVersionFailed.Broadcast(Message, ErrorCode);
			}
		},
		[ThisPtr]
		(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled())
			{
				ThisPtr->OnDeleteVersionSucceed.Broadcast(Message);
			}
		},
		Input.bUseCancellationToken,
		(Input.CancellationToken && Input.CancellationToken->IsValidLowLevel() && !Input.CancellationToken->IsPendingKillOrUnreachable()) ? Input.CancellationToken->GetData() : nullptr);
}

void UBKActionDeleteVersion::Perform_DeleteRevision_Action_BP(
	const FBKActionDeleteVersionInput& Input,
	BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
	FBKActionDeleteVersionOutput& Output,
	struct FLatentActionInfo LatentInfo)
{
	UBKActionDeleteVersion* Action = NewObject<UBKActionDeleteVersion>(UBKManagerStore::GetActionManager());
	Action->Input = Input;
	Action->Output.RelativeInput = Action->Input;
	Action->Output.InputPtr = &Action->Input;
	Action->SetInputOutputPointers((FBKInputBase*)&Input, (FBKOutputBase*)&Output);

	Action->CalledFromBP(Exec, LatentInfo, &Action->Output, &Output);

	Action->OnDeleteVersionFailed.AddWeakLambda(Action, [Action]
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
	Action->OnDeleteVersionSucceed.AddWeakLambda(Action, [Action]
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

void UBKActionDeleteVersion::Perform_DeleteVersion_Action_Cpp(
	const FBKActionDeleteVersionInput& Input,
	const TFunction<void(const FString&, int32)>& FailureCallback,
	const TFunction<void(const FString&)>& SuccessCallback)
{
	UBKActionDeleteVersion* Action = NewObject<UBKActionDeleteVersion>(UBKManagerStore::GetActionManager());
	Action->Input = Input;
	Action->SetInputOutputPointers((FBKInputBase*)&Input, nullptr);

	Action->OnDeleteVersionFailed.AddWeakLambda(Action, [Action, FailureCallback]
		(const FString& Message, int32 ErrorCode)
		{
			FailureCallback(Message, ErrorCode);
			Action->ActionCompleted();
		});
	Action->OnDeleteVersionSucceed.AddWeakLambda(Action, [Action, SuccessCallback]
		(const FString& Message)
		{
			SuccessCallback(Message);
			Action->ActionCompleted();
		});

	UBKManagerStore::GetActionManager()->QueueAction(Action);
}
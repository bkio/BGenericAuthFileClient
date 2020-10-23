/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "Actions/BActionCreateRevision.h"
#include "Managers/BManagerStore.h"
#include "Managers/BActionManager.h"
#include "Managers/BNetworkManager.h"
#include "Network/BHTTPWrapper.h"
#include "Network/BCancellationToken.h"
#include "Engine/LatentActionManager.h"
#include "JsonUtilities.h"

void UBKActionCreateRevision::Execute()
{
	Super::Execute();

	FString Url = BASE_PATH;
	Url.Append("file/models/" + Input.ModelId + "/revisions");

	TWeakObjectPtr<UBKActionCreateRevision> ThisPtr(this);

	UBKManagerStore::GetNetworkManager()->Put(Url, JsonString,
		[ThisPtr]
		(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message, int32 ErrorCode)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled())
			{
				ThisPtr->OnCreateRevisionFailed.Broadcast(Message, ErrorCode);
			}
		},
		[ThisPtr]
		(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled() && HttpWrapper.IsValid())
			{
				HttpWrapper.Pin()->GetResponseObject(ThisPtr->JsonObj);
				ThisPtr->OnCreateRevisionSucceed.Broadcast(ThisPtr->JsonObj->GetIntegerField("revisionIndex"));
			}
		},
		Input.bUseCancellationToken,
		(Input.CancellationToken && Input.CancellationToken->IsValidLowLevel() && !Input.CancellationToken->IsPendingKillOrUnreachable()) ? Input.CancellationToken->GetData() : nullptr);
}

void UBKActionCreateRevision::Perform_CreateRevision_Action_BP(
	const FBKActionCreateRevisionInput& Input,
	BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
	FBKActionCreateRevisionOutput& Output,
	FLatentActionInfo LatentInfo)
{
	UBKActionCreateRevision* Action = NewObject<UBKActionCreateRevision>(UBKManagerStore::GetActionManager());
	Action->Input = Input;
	Action->Output.RelativeInput = Action->Input;
	Action->Output.InputPtr = &Action->Input;
	
	Action->InitializeActionDefinition(FModelRevision::StaticStruct(), Input.RevisionInfo)
		->ThisIsAnUpdateAction(&Action->Input.RevisionInfo)
		->CalledFromBP(Exec, LatentInfo, &Action->Output, &Output)
		->FinalizeActionDefinition();

	Action->OnCreateRevisionFailed.AddWeakLambda(Action, [Action]
		(const FString& Message, int32 ErrorCode)
		{
			if (Action->Output.Exec)
				*Action->Output.Exec = BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL::Failed;

			Action->Output.ErrorMessage = Message;
			Action->Output.ErrorCode = ConvertHttpCodeToActionErrorCode(ErrorCode);

			if (Action->Output.Condition.IsValid())
				*Action->Output.Condition.Get() = true;

			Action->ActionCompleted();
		});
	Action->OnCreateRevisionSucceed.AddWeakLambda(Action, [Action]
		(int32 RevisionIndex)
		{
			if (Action->Output.Exec)
				*Action->Output.Exec = BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL::Succeed;

			Action->Output.RevisionIndex = RevisionIndex;

			if (Action->Output.Condition.IsValid())
				*Action->Output.Condition.Get() = true;

			Action->ActionCompleted();
		});

	UBKManagerStore::GetActionManager()->QueueAction(Action);
}

void UBKActionCreateRevision::Perform_CreateRevision_Action_Cpp(
	const FBKActionCreateRevisionInput& Input,
	const TFunction<void(const FString&, int32)>& FailureCallback,
	const TFunction<void(int32)>& SuccessCallback)
{
	UBKActionCreateRevision* Action = NewObject<UBKActionCreateRevision>(UBKManagerStore::GetActionManager());
	Action->Input = Input;

	Action->InitializeActionDefinition(FModelRevision::StaticStruct(), Action->Input.RevisionInfo)
		->ThisIsAnUpdateAction(&Action->Input.RevisionInfo)
		->FinalizeActionDefinition();

	Action->OnCreateRevisionFailed.AddWeakLambda(Action, [Action, FailureCallback](const FString& Message, int32 ErrorCode)
		{
			FailureCallback(Message, ErrorCode);
			Action->ActionCompleted();
		});
	Action->OnCreateRevisionSucceed.AddWeakLambda(Action, [Action, SuccessCallback](int32 RevisionIndex)
		{
			SuccessCallback(RevisionIndex);
			Action->ActionCompleted();
		});

	UBKManagerStore::GetActionManager()->QueueAction(Action);
}
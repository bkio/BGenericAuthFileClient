/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "Actions/BActionUpdateRevisionInfo.h"
#include "Managers/BManagerStore.h"
#include "Managers/BActionManager.h"
#include "Managers/BNetworkManager.h"
#include "Network/BHTTPWrapper.h"
#include "Engine/LatentActionManager.h"
#include "Network/BCancellationToken.h"
#include "JsonUtilities.h"

void UBKActionUpdateRevisionInfo::Execute()
{
	Super::Execute();

	FString Url = BASE_PATH;
	Url.Append("file/models/" + Input.ModelId + "/revisions/" + FString::FromInt(Input.RevisionIndex));

	TWeakObjectPtr<UBKActionUpdateRevisionInfo> ThisPtr(this);

	UBKManagerStore::GetNetworkManager()->Post(Url, JsonString,
		[ThisPtr]
		(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message, int32 ErrorCode)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled())
			{
				ThisPtr->OnUpdateRevisionInfoFailed.Broadcast(Message, ErrorCode);
			}
		},
		[ThisPtr]
		(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled())
			{
				ThisPtr->OnUpdateRevisionInfoSucceed.Broadcast(Message);
			}
		},
		Input.bUseCancellationToken,
		(Input.CancellationToken && Input.CancellationToken->IsValidLowLevel() && !Input.CancellationToken->IsPendingKillOrUnreachable()) ? Input.CancellationToken->GetData() : nullptr);
}

void UBKActionUpdateRevisionInfo::Perform_UpdateRevisionInfo_Action_BP(
	const FBKActionUpdateRevisionInfoInput& Input,
	BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
	FBKActionUpdateRevisionInfoOutput& Output,
	struct FLatentActionInfo LatentInfo)
{
	UBKActionUpdateRevisionInfo* Action = NewObject<UBKActionUpdateRevisionInfo>(UBKManagerStore::GetActionManager());
	FBKActionUpdateRevisionInfoInput NewInput;
	NewInput.ActionId = Input.ActionId;
	NewInput.bUseCancellationToken = Input.bUseCancellationToken;
	NewInput.CancellationToken = Input.CancellationToken;
	NewInput.ModelId = Input.ModelId;
	NewInput.RevisionIndex = Input.RevisionIndex;
	NewInput.RevisionInfo = Input.RevisionInfo;

	Action->Input = NewInput;
	Action->Output.RelativeInput = Action->Input;
	Action->Output.InputPtr = &Action->Input;
	Action->SetInputOutputPointers((FBKInputBase*)&Action->Input, (FBKOutputBase*)&Action->Output);

	Action->InitializeActionDefinition(FModelRevision::StaticStruct(), Input.RevisionInfo)
		->ThisIsAnUpdateAction(&Action->Input.RevisionInfo)
		->CalledFromBP(Exec, LatentInfo, &Action->Output, &Output)
		->FinalizeActionDefinition();

	Action->OnUpdateRevisionInfoFailed.AddWeakLambda(Action, [Action]
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
	Action->OnUpdateRevisionInfoSucceed.AddWeakLambda(Action, [Action]
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

void UBKActionUpdateRevisionInfo::Perform_UpdateRevisionInfo_Action_Cpp(
	const FBKActionUpdateRevisionInfoInput& Input,
	const TFunction<void(const FString&, int32)>& FailureCallback,
	const TFunction<void(const FString&)>& SuccessCallback)
{
	UBKActionUpdateRevisionInfo* Action = NewObject<UBKActionUpdateRevisionInfo>(UBKManagerStore::GetActionManager());
	Action->Input = Input;
	Action->SetInputOutputPointers((FBKInputBase*)&Input, nullptr);

	Action->InitializeActionDefinition(FModelRevision::StaticStruct(), Input.RevisionInfo)
		->ThisIsAnUpdateAction(&Action->Input.RevisionInfo)
		->FinalizeActionDefinition();

	Action->OnUpdateRevisionInfoFailed.AddWeakLambda(Action, [Action, FailureCallback](const FString& Message, int32 ErrorCode)
		{
			FailureCallback(Message, ErrorCode);
			Action->ActionCompleted();
		});
	Action->OnUpdateRevisionInfoSucceed.AddWeakLambda(Action, [Action, SuccessCallback](const FString& Message)
		{
			SuccessCallback(Message);
			Action->ActionCompleted();
		});

	UBKManagerStore::GetActionManager()->QueueAction(Action);
}
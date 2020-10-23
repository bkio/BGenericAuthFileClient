/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "Actions/BActionRemoveSharedWithUser.h"
#include "Managers/BManagerStore.h"
#include "Managers/BActionManager.h"
#include "Managers/BNetworkManager.h"
#include "Network/BHTTPWrapper.h"
#include "Engine/LatentActionManager.h"
#include "JsonUtilities.h"
#include "BZipFile.h"
#include "BLambdaRunnable.h"
#include "Network/BCancellationToken.h"

void UBKActionDeleteSharedModel::Execute()
{
	Super::Execute();

	FString Url = BASE_PATH;
	Url.Append("file/models/" + Input.ModelId + "/remove_sharing_from/user_id/" + Input.UserId);

	TWeakObjectPtr<UBKActionDeleteSharedModel> ThisPtr(this);

	UBKManagerStore::GetNetworkManager()->Delete(Url,
		[ThisPtr]
	(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message, int32 ErrorCode)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled())
			{
				ThisPtr->OnDeleteSharedModelFailed.Broadcast(Message, ErrorCode);
			}
		},
		[ThisPtr]
		(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled())
			{
				ThisPtr->OnDeleteSharedModelSucceed.Broadcast();
			}
		},
			Input.bUseCancellationToken,
			(Input.CancellationToken && Input.CancellationToken->IsValidLowLevel() && !Input.CancellationToken->IsPendingKillOrUnreachable()) ? Input.CancellationToken->GetData() : nullptr);
}


void UBKActionDeleteSharedModel::Perform_RemoveSharedModel_Action_BP(
	const FBKActionRemoveSharedModelInput& Input,
	BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
	FBKActionDeleteSharedModelOutput& Output,
	struct FLatentActionInfo LatentInfo)
{
	UBKActionDeleteSharedModel* Action = NewObject<UBKActionDeleteSharedModel>(UBKManagerStore::GetActionManager());

	FBKActionRemoveSharedModelInput NewInput;
	NewInput.ActionId = Input.ActionId;
	NewInput.bUseCancellationToken = Input.bUseCancellationToken;
	NewInput.CancellationToken = Input.CancellationToken;
	NewInput.UserId = Input.UserId;
	NewInput.ModelId = Input.ModelId;

	Action->Input = NewInput;

	Action->Output.ModelId = NewInput.ModelId;
	Action->Output.RelativeInput = NewInput;
	Action->Output.InputPtr = &Action->Input;
	Action->SetInputOutputPointers((FBKInputBase*)&Action->Input, (FBKOutputBase*)&Action->Output);

	
	Action->CalledFromBP(Exec, LatentInfo, &Action->Output, &Output);

	Action->OnDeleteSharedModelFailed.AddWeakLambda(Action, [Action]
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
	Action->OnDeleteSharedModelSucceed.AddWeakLambda(Action, [Action]
	()
		{
			if (Action->Output.Exec)
				*Action->Output.Exec = BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL::Succeed;

			Action->Output.ModelId = Action->Output.RelativeInput.ModelId;
			Action->Output.UserId = Action->Output.RelativeInput.UserId;

			if (Action->Output.Condition.IsValid())
				*Action->Output.Condition.Get() = true;

			Action->ActionCompleted();
		});

	UBKManagerStore::GetActionManager()->QueueAction(Action);
}
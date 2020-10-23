/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "Actions/BActionShareModel.h"
#include "Managers/BManagerStore.h"
#include "Managers/BActionManager.h"
#include "Managers/BNetworkManager.h"
#include "Network/BHTTPWrapper.h"
#include "Engine/LatentActionManager.h"
#include "JsonUtilities.h"
#include "BZipFile.h"
#include "BLambdaRunnable.h"
#include "Network/BCancellationToken.h"

void UBKActionShareModel::Execute()
{
	Super::Execute();

	FString Url = BASE_PATH;
	Url.Append("file/models/" + Input.ModelId + "/sharing");

	TWeakObjectPtr<UBKActionShareModel> ThisPtr(this);

	UBKManagerStore::GetNetworkManager()->Post(Url, JsonString,
		[ThisPtr]
	(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message, int32 ErrorCode)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled())
			{
				ThisPtr->OnShareModelFailed.Broadcast(Message, ErrorCode);
			}
		},
		[ThisPtr]
		(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled())
			{
				ThisPtr->OnShareModelSucceed.Broadcast(Message);
			}
		},
			Input.bUseCancellationToken,
			(Input.CancellationToken && Input.CancellationToken->IsValidLowLevel() && !Input.CancellationToken->IsPendingKillOrUnreachable()) ? Input.CancellationToken->GetData() : nullptr);
}


void UBKActionShareModel::Perform_ShareModel_Action_BP(
	const FBKActionShareModelInput& Input,
	BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
	FBKActionShareModelOutput& Output,
	struct FLatentActionInfo LatentInfo)
{
	UBKActionShareModel* Action = NewObject<UBKActionShareModel>(UBKManagerStore::GetActionManager());

	FBKActionShareModelInput NewInput;
	NewInput.ActionId = Input.ActionId;
	NewInput.bUseCancellationToken = Input.bUseCancellationToken;
	NewInput.CancellationToken = Input.CancellationToken;
	NewInput.ShareInfo = Input.ShareInfo;
	NewInput.ModelId = Input.ModelId;

	Action->Input = NewInput;

	Action->Output.ModelId = NewInput.ModelId;
	Action->Output.RelativeInput = NewInput;
	Action->Output.InputPtr = &Action->Input;
	Action->SetInputOutputPointers((FBKInputBase*)&Action->Input, (FBKOutputBase*)&Action->Output);

	
	Action->InitializeActionDefinition(FShareInfo::StaticStruct(), Input.ShareInfo)
		->ThisIsAnUpdateAction(&Action->Input.ShareInfo)
		->CalledFromBP(Exec, LatentInfo, &Action->Output, &Output)
		->FinalizeActionDefinition();

	Action->OnShareModelFailed.AddWeakLambda(Action, [Action]
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
	Action->OnShareModelSucceed.AddWeakLambda(Action, [Action]
	(const FString& ModelId)
		{
			if (Action->Output.Exec)
				*Action->Output.Exec = BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL::Succeed;

			Action->Output.ModelId = ModelId;

			if (Action->Output.Condition.IsValid())
				*Action->Output.Condition.Get() = true;

			Action->ActionCompleted();
		});

	UBKManagerStore::GetActionManager()->QueueAction(Action);
}
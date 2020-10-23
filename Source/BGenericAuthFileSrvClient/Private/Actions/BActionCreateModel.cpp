/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "Actions/BActionCreateModel.h"
#include "Managers/BManagerStore.h"
#include "Managers/BActionManager.h"
#include "Managers/BNetworkManager.h"
#include "Network/BHTTPWrapper.h"
#include "Network/BCancellationToken.h"
#include "Engine/LatentActionManager.h"
#include "JsonUtilities.h"

void UBKActionCreateModel::Execute()
{
	Super::Execute();

	FString Url = BASE_PATH;
	Url.Append("file/models");

	TWeakObjectPtr<UBKActionCreateModel> ThisPtr(this);

	UBKManagerStore::GetNetworkManager()->Put(Url, JsonString,
		[ThisPtr]
		(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message, int32 ErrorCode)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled())
			{
				ThisPtr->OnCreateModelFailed.Broadcast(Message, ErrorCode);
			}
		},
		[ThisPtr]
		(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled() && HttpWrapper.IsValid())
			{
				HttpWrapper.Pin()->GetResponseObject(ThisPtr->JsonObj);
				ThisPtr->OnCreateModelSucceed.Broadcast(ThisPtr->JsonObj->GetStringField("modelId"));
			}
		},
		Input.bUseCancellationToken,
		(Input.CancellationToken && Input.CancellationToken->IsValidLowLevel() && !Input.CancellationToken->IsPendingKillOrUnreachable()) ? Input.CancellationToken->GetData() : nullptr);
}

void UBKActionCreateModel::Perform_CreateModel_Action_BP(
	const FBKActionCreateModelInput& Input,
	BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
	FBKActionCreateModelOutput& Output,
	FLatentActionInfo LatentInfo)
{
	UBKActionCreateModel* Action = NewObject<UBKActionCreateModel>(UBKManagerStore::GetActionManager());
	Action->Input = Input;
	Action->Output.RelativeInput = Action->Input;
	Action->Output.InputPtr = &Action->Input;
	Action->SetInputOutputPointers((FBKInputBase*)&Input, (FBKOutputBase*)&Output);
	
	Action->InitializeActionDefinition(FModelInfo::StaticStruct(), Input.ModelInfo)
		->ThisIsAnUpdateAction(&Action->Input.ModelInfo)
		->CalledFromBP(Exec, LatentInfo, &Action->Output, &Output)
		->FinalizeActionDefinition();

	Action->OnCreateModelFailed.AddWeakLambda(Action, [Action]
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
	Action->OnCreateModelSucceed.AddWeakLambda(Action, [Action]
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

void UBKActionCreateModel::Perform_CreateModel_Action_Cpp(
	const FBKActionCreateModelInput& Input,
	const TFunction<void(const FString&, int32)>& FailureCallback,
	const TFunction<void(const FString&)>& SuccessCallback)
{
	UBKActionCreateModel* Action = NewObject<UBKActionCreateModel>(UBKManagerStore::GetActionManager());
	Action->Input = Input;
	Action->SetInputOutputPointers((FBKInputBase*)&Input, nullptr);

	Action->InitializeActionDefinition(FModelInfo::StaticStruct(), Input.ModelInfo)
		->ThisIsAnUpdateAction(&Action->Input.ModelInfo)
		->FinalizeActionDefinition();

	Action->OnCreateModelFailed.AddWeakLambda(Action, [Action, FailureCallback]
		(const FString& Message, int32 ErrorCode)
		{
			FailureCallback(Message, ErrorCode);
			Action->ActionCompleted();
		});
	Action->OnCreateModelSucceed.AddWeakLambda(Action, [Action, SuccessCallback]
		(const FString& ModelId)
		{
			SuccessCallback(ModelId);
			Action->ActionCompleted();
		});

	UBKManagerStore::GetActionManager()->QueueAction(Action);
}
/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "Actions/BActionGetModelInfo.h"
#include "Managers/BManagerStore.h"
#include "Managers/BActionManager.h"
#include "Managers/BNetworkManager.h"
#include "Network/BHTTPWrapper.h"
#include "Network/BCancellationToken.h"
#include "Engine/LatentActionManager.h"
#include "JsonUtilities.h"

void UBKActionGetModelInfo::Execute()
{
	Super::Execute();

	FString Url = BASE_PATH;
	Url.Append(END_POINT_FILE_MODELS + Input.ModelId);

	TWeakObjectPtr<UBKActionGetModelInfo> ThisPtr(this);

	UBKManagerStore::GetNetworkManager()->Get(Url,
		[ThisPtr]
		(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message, int32 ErrorCode)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled())
			{
				ThisPtr->OnGetModelInfoFailed.Broadcast(Message, ErrorCode);
			}
		},
		[ThisPtr]
		(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled() && HttpWrapper.IsValid())
			{
				FModelInfo ModelInfo;
				HttpWrapper.Pin()->GetResponseObject(ThisPtr->JsonObj);
				FJsonObjectConverter::JsonObjectStringToUStruct<FModelInfo>(Message, &ModelInfo, 0, 0);

				ThisPtr->OnGetModelInfoSucceed.Broadcast(ModelInfo);
			}
		},
		Input.bUseCancellationToken,
		(Input.CancellationToken && Input.CancellationToken->IsValidLowLevel() && !Input.CancellationToken->IsPendingKillOrUnreachable()) ? Input.CancellationToken->GetData() : nullptr);
}

void UBKActionGetModelInfo::Perform_GetModelInfo_Action_BP(
	const FBKActionGetModelInfoInput& Input,
	BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
	FBKActionGetModelInfoOutput& Output,
	FLatentActionInfo LatentInfo)
{
	UBKActionGetModelInfo* Action = NewObject<UBKActionGetModelInfo>();
	Action->Input = Input;
	Action->Output.RelativeInput = Action->Input;
	Action->Output.InputPtr = &Action->Input;
	Action->SetInputOutputPointers((FBKInputBase*)&Input, (FBKOutputBase*)&Output);

	Action->CalledFromBP(Exec, LatentInfo, &Action->Output, &Output);

	Action->OnGetModelInfoFailed.AddWeakLambda(Action, [Action]
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
	Action->OnGetModelInfoSucceed.AddWeakLambda(Action, [Action]
		(const FModelInfo& ModelInfo)
		{
			if (Action->Output.Exec)
				*Action->Output.Exec = BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL::Succeed;

			Action->Output.Model = ModelInfo;

			if (Action->Output.Condition.IsValid())
				*Action->Output.Condition.Get() = true;

			Action->ActionCompleted();
		});

	UBKManagerStore::GetActionManager()->QueueAction(Action);
}

void UBKActionGetModelInfo::Perform_GetModelInfo_Action_Cpp(
	const FBKActionGetModelInfoInput& Input,
	const TFunction<void(const FString&, int32)>& FailureCallback,
	const TFunction<void(const FModelInfo&)>& SuccessCallback)
{
	UBKActionGetModelInfo* Action = NewObject<UBKActionGetModelInfo>();
	Action->Input = Input;
	Action->SetInputOutputPointers((FBKInputBase*)&Input, nullptr);

	Action->OnGetModelInfoFailed.AddWeakLambda(Action, [Action, FailureCallback]
		(const FString& Message, int32 ErrorCode)
		{
			FailureCallback(Message, ErrorCode);
			Action->ActionCompleted();
		});
	Action->OnGetModelInfoSucceed.AddWeakLambda(Action, [Action, SuccessCallback]
		(const FModelInfo& Model)
		{
			SuccessCallback(Model);
			Action->ActionCompleted();
		});

	UBKManagerStore::GetActionManager()->QueueAction(Action);
}
/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "Actions/BActionUpdateModelInfo.h"
#include "Managers/BManagerStore.h"
#include "Managers/BActionManager.h"
#include "Managers/BNetworkManager.h"
#include "Network/BHTTPWrapper.h"
#include "Engine/LatentActionManager.h"
#include "Network/BCancellationToken.h"
#include "JsonUtilities.h"

void UBKActionUpdateModelInfo::Execute()
{
	Super::Execute();

	FString Url = BASE_PATH;
	Url.Append("file/models/" + Input.ModelId);

	TWeakObjectPtr<UBKActionUpdateModelInfo> ThisPtr(this);

	UBKManagerStore::GetNetworkManager()->Post(Url, JsonString,
		[ThisPtr]
		(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message, int32 ErrorCode)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled())
			{
				ThisPtr->OnUpdateModelInfoFailed.Broadcast(Message, ErrorCode);
			}
		},
		[ThisPtr]
		(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled())
			{
				ThisPtr->OnUpdateModelInfoSucceed.Broadcast(Message);
			}
		},
		Input.bUseCancellationToken,
		(Input.CancellationToken && Input.CancellationToken->IsValidLowLevel() && !Input.CancellationToken->IsPendingKillOrUnreachable()) ? Input.CancellationToken->GetData() : nullptr);
}

void UBKActionUpdateModelInfo::Perform_UpdateModelInfo_Action_BP(
	const FBKActionUpdateModelInfoInput& Input,
	BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
	FBKActionUpdateModelInfoOutput& Output,
	struct FLatentActionInfo LatentInfo)
{
	UBKActionUpdateModelInfo* Action = NewObject<UBKActionUpdateModelInfo>(UBKManagerStore::GetActionManager());

	FBKActionUpdateModelInfoInput NewInput;
	NewInput.ActionId = Input.ActionId;
	NewInput.bUseCancellationToken = Input.bUseCancellationToken;
	NewInput.CancellationToken = Input.CancellationToken;
	NewInput.ModelId = Input.ModelId;
	NewInput.ModelInfo = Input.ModelInfo;

	Action->Input = NewInput;
	Action->Output.RelativeInput = Action->Input;
	Action->Output.InputPtr = &Action->Input;
	Action->SetInputOutputPointers((FBKInputBase*)&Action->Input, (FBKOutputBase*)&Action->Output);

	Action->InitializeActionDefinition(FModelInfo::StaticStruct(), Input.ModelInfo)
		->ThisIsAnUpdateAction(&Action->Input.ModelInfo)
		->CalledFromBP(Exec, LatentInfo, &Action->Output, &Output)
		->FinalizeActionDefinition();

	Action->OnUpdateModelInfoFailed.AddWeakLambda(Action, [Action]
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
	Action->OnUpdateModelInfoSucceed.AddWeakLambda(Action, [Action]
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

void UBKActionUpdateModelInfo::Perform_UpdateModelInfo_Action_Cpp(
	const FBKActionUpdateModelInfoInput& Input,
	const TFunction<void(const FString&, int32)>& FailureCallback,
	const TFunction<void(const FString&)>& SuccessCallback)
{
	UBKActionUpdateModelInfo* Action = NewObject<UBKActionUpdateModelInfo>(UBKManagerStore::GetActionManager());
	Action->Input = Input;
	Action->SetInputOutputPointers((FBKInputBase*)&Input, nullptr);

	Action->InitializeActionDefinition(FModelInfo::StaticStruct(), Input.ModelInfo)
		->ThisIsAnUpdateAction(&Action->Input.ModelInfo)
		->FinalizeActionDefinition();

	Action->OnUpdateModelInfoFailed.AddWeakLambda(Action, [Action, FailureCallback](const FString& Message, int32 ErrorCode)
		{
			FailureCallback(Message, ErrorCode);
			Action->ActionCompleted();
		});
	Action->OnUpdateModelInfoSucceed.AddWeakLambda(Action, [Action, SuccessCallback](const FString& Message)
		{
			SuccessCallback(Message);
			Action->ActionCompleted();
		});

	UBKManagerStore::GetActionManager()->QueueAction(Action);
}
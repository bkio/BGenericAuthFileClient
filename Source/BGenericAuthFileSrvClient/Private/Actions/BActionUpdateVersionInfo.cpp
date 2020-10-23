/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "Actions/BActionUpdateVersionInfo.h"
#include "Managers/BManagerStore.h"
#include "Managers/BActionManager.h"
#include "Managers/BNetworkManager.h"
#include "Network/BHTTPWrapper.h"
#include "Engine/LatentActionManager.h"
#include "Network/BCancellationToken.h"
#include "JsonUtilities.h"

void UBKActionUpdateVersionInfo::Execute()
{
	Super::Execute();

	FString Url = BASE_PATH;
	Url.Append("file/models/" + Input.ModelId + "/revisions/" + FString::FromInt(Input.RevisionIndex) + "/versions/" + FString::FromInt(Input.VersionIndex));

	TWeakObjectPtr<UBKActionUpdateVersionInfo> ThisPtr(this);

	UBKManagerStore::GetNetworkManager()->Post(Url, JsonString,
		[ThisPtr]
		(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message, int32 ErrorCode)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled())
			{
				ThisPtr->OnUpdateVersionInfoFailed.Broadcast(Message, ErrorCode);
			}
		},
		[ThisPtr]
		(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled())
			{
				ThisPtr->OnUpdateVersionInfoSucceed.Broadcast(Message);
			}
		},
		Input.bUseCancellationToken,
		(Input.CancellationToken && Input.CancellationToken->IsValidLowLevel() && !Input.CancellationToken->IsPendingKillOrUnreachable()) ? Input.CancellationToken->GetData() : nullptr);
}

void UBKActionUpdateVersionInfo::Perform_UpdateVersionInfo_Action_BP(
	const FBKActionUpdateVersionInfoInput& Input,
	BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
	FBKActionUpdateVersionInfoOutput& Output,
	struct FLatentActionInfo LatentInfo)
{
	UBKActionUpdateVersionInfo* Action = NewObject<UBKActionUpdateVersionInfo>(UBKManagerStore::GetActionManager());
	
	FBKActionUpdateVersionInfoInput NewInput;
	NewInput.ActionId = Input.ActionId;
	NewInput.bUseCancellationToken = Input.bUseCancellationToken;
	NewInput.CancellationToken = Input.CancellationToken;
	NewInput.ModelId = Input.ModelId;
	NewInput.RevisionIndex = Input.RevisionIndex;
	NewInput.VersionIndex = Input.VersionIndex;
	NewInput.VersionInfo = Input.VersionInfo;

	Action->Input = NewInput;
	Action->Output.RelativeInput = Action->Input;
	Action->Output.InputPtr = &Action->Input;
	Action->SetInputOutputPointers((FBKInputBase*)&Action->Input, (FBKOutputBase*)&Action->Output);

	Action->InitializeActionDefinition(FModelRevisionVersion::StaticStruct(), Input.VersionInfo)
		->ThisIsAnUpdateAction(&Action->Input.VersionInfo)
		->CalledFromBP(Exec, LatentInfo, &Action->Output, &Output)
		->FinalizeActionDefinition();

	Action->OnUpdateVersionInfoFailed.AddWeakLambda(Action, [Action]
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
	Action->OnUpdateVersionInfoSucceed.AddWeakLambda(Action, [Action]
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

void UBKActionUpdateVersionInfo::Perform_UpdateVersionInfo_Action_Cpp(
	const FBKActionUpdateVersionInfoInput& Input,
	const TFunction<void(const FString&, int32)>& FailureCallback,
	const TFunction<void(const FString&)>& SuccessCallback)
{
	UBKActionUpdateVersionInfo* Action = NewObject<UBKActionUpdateVersionInfo>(UBKManagerStore::GetActionManager());
	Action->Input = Input;
	Action->SetInputOutputPointers((FBKInputBase*)&Input, nullptr);

	Action->InitializeActionDefinition(FModelRevisionVersion::StaticStruct(), Input.VersionInfo)
		->ThisIsAnUpdateAction(&Action->Input.VersionInfo)
		->FinalizeActionDefinition();

	Action->OnUpdateVersionInfoFailed.AddWeakLambda(Action, [Action, FailureCallback](const FString& Message, int32 ErrorCode)
		{
			FailureCallback(Message, ErrorCode);
			Action->ActionCompleted();
		});
	Action->OnUpdateVersionInfoSucceed.AddWeakLambda(Action, [Action, SuccessCallback](const FString& Message)
		{
			SuccessCallback(Message);
			Action->ActionCompleted();
		});

	UBKManagerStore::GetActionManager()->QueueAction(Action);
}
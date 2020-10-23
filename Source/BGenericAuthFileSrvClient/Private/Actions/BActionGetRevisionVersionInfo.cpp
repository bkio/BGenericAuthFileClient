/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "Actions/BActionGetRevisionVersionInfo.h"
#include "Managers/BManagerStore.h"
#include "Managers/BActionManager.h"
#include "Managers/BNetworkManager.h"
#include "Network/BHTTPWrapper.h"
#include "Network/BCancellationToken.h"
#include "Engine/LatentActionManager.h"
#include "JsonUtilities.h"

void UBKActionGetRevisionVersionInfo::Execute()
{
	Super::Execute();

	FString Url = BASE_PATH;
	Url.Append(END_POINT_FILE_MODELS + Input.ModelId + "/revisions/" + FString::FromInt(Input.RevisionIndex) + "/versions/" + FString::FromInt(Input.VersionIndex));

	TWeakObjectPtr<UBKActionGetRevisionVersionInfo> ThisPtr(this);

	UBKManagerStore::GetNetworkManager()->Get(Url,
		[ThisPtr]
		(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message, int32 ErrorCode)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled())
			{
				ThisPtr->OnGetRevisionVersionInfoFailed.Broadcast(Message, ErrorCode);
			}
		},
		[ThisPtr]
		(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled() && HttpWrapper.IsValid())
			{
				FModelRevisionVersion Version;
				HttpWrapper.Pin()->GetResponseObject(ThisPtr->JsonObj);
				FJsonObjectConverter::JsonObjectStringToUStruct<FModelRevisionVersion>(Message, &Version, 0, 0);

				ThisPtr->OnGetRevisionVersionInfoSucceed.Broadcast(Version);
			}
		},
		Input.bUseCancellationToken,
		(Input.CancellationToken && Input.CancellationToken->IsValidLowLevel() && !Input.CancellationToken->IsPendingKillOrUnreachable()) ? Input.CancellationToken->GetData() : nullptr);
}

void UBKActionGetRevisionVersionInfo::Perform_GetRevisionVersionInfo_Action_BP(
	const FBKActionGetRevisionVersionInfoInput& Input,
	BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
	FBKActionGetRevisionVersionInfoOutput& Output,
	FLatentActionInfo LatentInfo)
{
	UBKActionGetRevisionVersionInfo* Action = NewObject<UBKActionGetRevisionVersionInfo>();
	Action->Input = Input;
	Action->Output.RelativeInput = Action->Input;
	Action->Output.InputPtr = &Action->Input;
	Action->SetInputOutputPointers((FBKInputBase*)&Input, (FBKOutputBase*)&Output);

	Action->CalledFromBP(Exec, LatentInfo, &Action->Output, &Output);

	Action->OnGetRevisionVersionInfoFailed.AddWeakLambda(Action, [Action]
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
	Action->OnGetRevisionVersionInfoSucceed.AddWeakLambda(Action, [Action]
		(const FModelRevisionVersion& Version)
		{
			if (Action->Output.Exec)
				*Action->Output.Exec = BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL::Succeed;

			Action->Output.Version = Version;

			if (Action->Output.Condition.IsValid())
				*Action->Output.Condition.Get() = true;

			Action->ActionCompleted();
		});

	UBKManagerStore::GetActionManager()->QueueAction(Action);
}

void UBKActionGetRevisionVersionInfo::Perform_GetRevisionVersionInfo_Action_Cpp(
	const FBKActionGetRevisionVersionInfoInput& Input,
	const TFunction<void(const FString&, int32)>& FailureCallback,
	const TFunction<void(const FModelRevisionVersion&)>& SuccessCallback)
{
	UBKActionGetRevisionVersionInfo* Action = NewObject<UBKActionGetRevisionVersionInfo>();
	Action->Input = Input;
	Action->SetInputOutputPointers((FBKInputBase*)&Input, nullptr);

	Action->OnGetRevisionVersionInfoFailed.AddWeakLambda(Action, [Action, FailureCallback]
	(const FString& Message, int32 ErrorCode)
		{
			FailureCallback(Message, ErrorCode);
			Action->ActionCompleted();
		});
	Action->OnGetRevisionVersionInfoSucceed.AddWeakLambda(Action, [Action, SuccessCallback]
	(const FModelRevisionVersion& Version)
		{
			SuccessCallback(Version);
			Action->ActionCompleted();
		});

	UBKManagerStore::GetActionManager()->QueueAction(Action);
}
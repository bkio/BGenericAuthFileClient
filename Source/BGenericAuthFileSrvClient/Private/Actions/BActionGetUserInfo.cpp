/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "Actions/BActionGetUserInfo.h"
#include "Managers/BManagerStore.h"
#include "Managers/BActionManager.h"
#include "Managers/BNetworkManager.h"
#include "Network/BHTTPWrapper.h"
#include "Network/BCancellationToken.h"
#include "Engine/LatentActionManager.h"
#include "JsonUtilities.h"

void UBKActionGetUserInfo::Execute()
{
	Super::Execute();

	FString UserId;
	if (!UBKManagerStore::GetNetworkManager()->GetCurrentUserId(UserId))
	{
		OnGetUserInfoFailed.Broadcast("User must authenticate to perform GetUserInfo call.", 401);
		return;
	}

	FString Url = BASE_PATH;
	Url.Append(END_POINT_AUTH_USER + UserId);

	TWeakObjectPtr<UBKActionGetUserInfo> ThisPtr(this);

	UBKManagerStore::GetNetworkManager()->Get(Url,
		[ThisPtr]
		(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message, int32 ErrorCode)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled())
			{
				ThisPtr->OnGetUserInfoFailed.Broadcast(Message, ErrorCode);
			}
		},
		[ThisPtr]
		(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled() && HttpWrapper.IsValid())
			{
				FUser UserInfo;
				HttpWrapper.Pin()->GetResponseObject(ThisPtr->JsonObj);
				FJsonObjectConverter::JsonObjectStringToUStruct<FUser>(Message, &UserInfo, 0, 0);

				ThisPtr->OnGetUserInfoSucceed.Broadcast(UserInfo);
			}
		},
		Input.bUseCancellationToken,
		(Input.CancellationToken && Input.CancellationToken->IsValidLowLevel() && !Input.CancellationToken->IsPendingKillOrUnreachable()) ? Input.CancellationToken->GetData() : nullptr);
}

void UBKActionGetUserInfo::Perform_GetUserInfo_Action_BP(
	const FBKInputBase& Input,
	BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
	FBKActionGetUserInfoOutput& Output,
	struct FLatentActionInfo LatentInfo)
{
	UBKActionGetUserInfo* Action = NewObject<UBKActionGetUserInfo>();
	Action->Input = Input;
	Action->Output.RelativeInput = Action->Input;
	Action->Output.InputPtr = &Action->Input;
	Action->SetInputOutputPointers((FBKInputBase*)&Input, (FBKOutputBase*)&Output);

	Action->CalledFromBP(Exec, LatentInfo, &Action->Output, &Output);

	Action->OnGetUserInfoFailed.AddWeakLambda(Action, [Action]
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
	Action->OnGetUserInfoSucceed.AddWeakLambda(Action, [Action]
		(const FUser& UserInfo)
		{
			if (Action->Output.Exec)
				*Action->Output.Exec = BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL::Succeed;

			Action->Output.UserInfo = UserInfo;

			if (Action->Output.Condition.IsValid())
				*Action->Output.Condition.Get() = true;

			Action->ActionCompleted();
		});

	UBKManagerStore::GetActionManager()->QueueAction(Action);
}

void UBKActionGetUserInfo::Perform_GetUserInfo_Action_Cpp(
	const TFunction<void(const FString&, int32)>& GetUserInfoFailedCallback,
	const TFunction<void(const FUser&)>& GetUserInfoSucceedCallback)
{
	UBKActionGetUserInfo* Action = NewObject<UBKActionGetUserInfo>();
	Action->SetInputOutputPointers(nullptr, nullptr);

	Action->OnGetUserInfoFailed.AddWeakLambda(Action, [Action, GetUserInfoFailedCallback]
	(const FString& Message, int32 ErrorCode)
		{
			GetUserInfoFailedCallback(Message, ErrorCode);
			Action->ActionCompleted();
		});
	Action->OnGetUserInfoSucceed.AddWeakLambda(Action, [Action, GetUserInfoSucceedCallback]
	(const FUser& User)
		{
			GetUserInfoSucceedCallback(User);
			Action->ActionCompleted();
		});

	UBKManagerStore::GetActionManager()->QueueAction(Action);
}
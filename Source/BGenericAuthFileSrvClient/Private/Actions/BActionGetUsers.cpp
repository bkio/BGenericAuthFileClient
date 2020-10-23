/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "Actions/BActionGetUsers.h"
#include "Managers/BManagerStore.h"
#include "Managers/BActionManager.h"
#include "Managers/BNetworkManager.h"
#include "Network/BHTTPWrapper.h"
#include "Engine/LatentActionManager.h"
#include "JsonUtilities.h"
#include "BZipFile.h"
#include "BLambdaRunnable.h"
#include "Network/BCancellationToken.h"

void UBKActionGetUsers::Execute()
{
	Super::Execute();

	FString Url = BASE_PATH;
	Url.Append("auth/list_registered_email_addresses");

	TWeakObjectPtr<UBKActionGetUsers> ThisPtr(this);

	UBKManagerStore::GetNetworkManager()->Get(Url,
	[ThisPtr]
	(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message, int32 ErrorCode)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled())
			{
				ThisPtr->OnGetUsersFailed.Broadcast(Message, ErrorCode);
			}
		},
		[ThisPtr]
		(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled())
			{
				FRegisteredEmailAddresses EmailAddresses;
				HttpWrapper.Pin()->GetResponseObject(ThisPtr->JsonObj);
				FJsonObjectConverter::JsonObjectStringToUStruct<FRegisteredEmailAddresses>(Message, &EmailAddresses, 0, 0);

				ThisPtr->OnGetUsersSucceed.Broadcast(EmailAddresses);
			}
		},
			Input.bUseCancellationToken,
			(Input.CancellationToken && Input.CancellationToken->IsValidLowLevel() && !Input.CancellationToken->IsPendingKillOrUnreachable()) ? Input.CancellationToken->GetData() : nullptr);
}

void UBKActionGetUsers::Perform_GetUserEmailAddresses_Action_BP(
	const FBKInputBase& Input,
	BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
	FBKActionGetUsersOutput& Output,
	struct FLatentActionInfo LatentInfo)
{
	UBKActionGetUsers* Action = NewObject<UBKActionGetUsers>(UBKManagerStore::GetActionManager());

	FBKInputBase NewInput;
	NewInput.ActionId = Input.ActionId;
	NewInput.bUseCancellationToken = Input.bUseCancellationToken;
	NewInput.CancellationToken = Input.CancellationToken;

	Action->Input = NewInput;
	Action->Output.RelativeInput = NewInput;
	Action->Output.InputPtr = &Action->Input;
	Action->SetInputOutputPointers((FBKInputBase*)&Action->Input, (FBKOutputBase*)&Action->Output);

	Action->CalledFromBP(Exec, LatentInfo, &Action->Output, &Output);

	Action->OnGetUsersFailed.AddWeakLambda(Action, [Action]
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
	Action->OnGetUsersSucceed.AddWeakLambda(Action, [Action]
	(const FRegisteredEmailAddresses& UserEmailAddresses)
		{
			if (Action->Output.Exec)
				*Action->Output.Exec = BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL::Succeed;

			Action->Output.EmailAddresses = UserEmailAddresses;

			if (Action->Output.Condition.IsValid())
				*Action->Output.Condition.Get() = true;

			Action->ActionCompleted();
		});

	UBKManagerStore::GetActionManager()->QueueAction(Action);
}
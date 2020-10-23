/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "Actions/BActionLogin.h"
#include "Managers/BManagerStore.h"
#include "Managers/BActionManager.h"
#include "Managers/BNetworkManager.h"
#include "Engine/LatentActionManager.h"
#include "JsonUtilities.h"
#include "Network/BCancellationToken.h"

void UBKActionLogin::Execute()
{
	Super::Execute();

	TWeakObjectPtr<UBKActionLogin> ThisPtr(this);

	UBKManagerStore::GetNetworkManager()->OnLoginFailed.AddWeakLambda(this, [ThisPtr]
		(const FString& Message)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled())
			{
				ThisPtr->OnLoginFailed.Broadcast(Message);
			}
		});
	UBKManagerStore::GetNetworkManager()->OnLoginSucceed.AddWeakLambda(this, [ThisPtr]
		(const FString& Message)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled())
			{
				ThisPtr->OnLoginSucceed.Broadcast(Message);
			}
		});

	if (!UBKManagerStore::GetNetworkManager()->StartLoginProcedure(
		Input.bUseCancellationToken, 
		(Input.CancellationToken && Input.CancellationToken->IsValidLowLevel() && !Input.CancellationToken->IsPendingKillOrUnreachable()) ? Input.CancellationToken->GetData() : nullptr))
	{
		ThisPtr->OnLoginFailed.Broadcast(TEXT("Could not start http server; check application permissions."));
	}
}

void UBKActionLogin::Perform_Login_Action_BP(
	const FBKInputBase& Input,
	BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
	FBKActionLoginOutput& Output,
	FLatentActionInfo LatentInfo)
{
	UBKActionLogin* Action = NewObject<UBKActionLogin>(UBKManagerStore::GetActionManager());
	Action->Input = Input;
	Action->Output.RelativeInput = Action->Input;
	Action->Output.InputPtr = &Action->Input;
	Action->SetInputOutputPointers((FBKInputBase*)&Input, (FBKOutputBase*)&Output);

	Action->CalledFromBP(Exec, LatentInfo, &Action->Output, &Output);

	Action->OnLoginFailed.AddWeakLambda(Action, [Action]
		(const FString& Message)
		{
			if (Action->Output.Exec)
				*Action->Output.Exec = BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL::Failed;

			Action->Output.Message = Message;

			if (Action->Output.Condition.IsValid())
				*Action->Output.Condition.Get() = true;

			Action->ActionCompleted();
		});
	Action->OnLoginSucceed.AddWeakLambda(Action, [Action]
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

void UBKActionLogin::Perform_Login_Action_Cpp(
	const TFunction<void(const FString&)>& LoginFailedCallback, 
	const TFunction<void(const FString&)>& LoginSucceedCallback)
{
	UBKActionLogin* Action = NewObject<UBKActionLogin>(UBKManagerStore::GetActionManager());
	Action->SetInputOutputPointers(nullptr, nullptr);

	Action->OnLoginFailed.AddWeakLambda(Action, [Action, LoginFailedCallback]
		(const FString& Message)
		{
			LoginFailedCallback(Message);
			Action->ActionCompleted();
		});
	Action->OnLoginSucceed.AddWeakLambda(Action, [Action, LoginSucceedCallback]
		(const FString& Message)
		{
			LoginSucceedCallback(Message);
			Action->ActionCompleted();
		});

	UBKManagerStore::GetActionManager()->QueueAction(Action);
}
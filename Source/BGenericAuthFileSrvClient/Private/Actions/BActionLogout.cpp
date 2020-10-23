/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "Actions/BActionLogout.h"
#include "JsonUtilities.h"
#include "Managers/BActionManager.h"
#include "Managers/BManagerStore.h"
#include "Managers/BNetworkManager.h"
#include "Engine/LatentActionManager.h"
#include "Network/BCancellationToken.h"

void UBKActionLogout::Execute()
{
	Super::Execute();

	if (IsActionCanceled()) return;

	UBKManagerStore::GetNetworkManager()->Logout();
	
	OnLogoutSucceed.Broadcast("Logout successful.");
}

void UBKActionLogout::Perform_Logout_Action_BP(
	const FBKInputBase& Input,
	BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
	FBKActionLogoutOutput& Output,
	FLatentActionInfo LatentInfo)
{
	UBKActionLogout* Action = NewObject<UBKActionLogout>(UBKManagerStore::GetActionManager());
	Action->Input = Input;
	Action->Output.RelativeInput = Action->Input;
	Action->Output.InputPtr = &Action->Input;
	Action->SetInputOutputPointers((FBKInputBase*)&Input, (FBKOutputBase*)&Output);

	Action->CalledFromBP(Exec, LatentInfo, &Action->Output, &Output);

	Action->OnLogoutFailed.AddWeakLambda(Action, [Action]
		(const FString& Message)
		{
			if (Action->Output.Exec)
				*Action->Output.Exec = BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL::Failed;

			Action->Output.Message = Message;

			if (Action->Output.Condition.IsValid())
				*Action->Output.Condition.Get() = true;

			Action->ActionCompleted();
		});
	Action->OnLogoutSucceed.AddWeakLambda(Action, [Action]
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

void UBKActionLogout::Perform_Logout_Action_Cpp(
	const TFunction<void(const FString&)>& LogoutFailedCallback,
	const TFunction<void(const FString&)>& LogoutSucceedCallback)
{
	UBKActionLogout* Action = NewObject<UBKActionLogout>(UBKManagerStore::GetActionManager());
	Action->SetInputOutputPointers(nullptr, nullptr);

	Action->OnLogoutFailed.AddWeakLambda(Action, [Action, LogoutFailedCallback]
		(const FString& Message)
		{
			LogoutFailedCallback(Message);
			Action->ActionCompleted();
		});
	Action->OnLogoutSucceed.AddWeakLambda(Action, [Action, LogoutSucceedCallback]
		(const FString& Message)
		{
			LogoutSucceedCallback(Message);
			Action->ActionCompleted();
		});

	UBKManagerStore::GetActionManager()->QueueAction(Action);
}
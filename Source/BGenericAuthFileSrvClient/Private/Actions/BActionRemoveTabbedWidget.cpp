/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "Actions/BActionRemoveTabbedWidget.h"
#include "Managers/BManagerStore.h"
#include "Managers/BViewManager.h"
#include "Managers/BActionManager.h"

void UBKActionRemoveTabbedWidget::Execute()
{
	Super::Execute();

	if (UBKViewManager* ViewManager = UBKManagerStore::GetViewManager())
	{
		TWeakObjectPtr<UBKActionRemoveTabbedWidget> ThisPtr(this);
		ViewManager->RemoveTabbedWidget(Input.OutUniqueId, [ThisPtr]
			(bool bSuccess, const FString& ErrorMessage)
			{
				if (!ThisPtr.IsValid() || ThisPtr->IsActionCanceled()) return;

				if (bSuccess)
				{
					ThisPtr->OnRemoveTabbedWidgetSucceed.Broadcast();
				}
				else
				{
					ThisPtr->OnRemoveTabbedWidgetFailed.Broadcast(ErrorMessage);
				}
			});
	}
}

void UBKActionRemoveTabbedWidget::Perform_RemoveTabbedWidget_Action_BP(
	const FBKActionRemoveTabbedWidgetInput& Input,
	BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
	FBKActionRemoveTabbedWidgetOutput& Output,
	struct FLatentActionInfo LatentInfo)
{
	UBKActionRemoveTabbedWidget* Action = NewObject<UBKActionRemoveTabbedWidget>(UBKManagerStore::GetActionManager());
	Action->Input = Input;
	Action->Output.RelativeInput = Action->Input;
	Action->Output.InputPtr = &Action->Input;
	Action->SetInputOutputPointers((FBKInputBase*)&Input, (FBKOutputBase*)&Output);

	Action->CalledFromBP(Exec, LatentInfo, &Action->Output, &Output);

	Action->OnRemoveTabbedWidgetFailed.AddWeakLambda(Action, [Action]
	(const FString& Message)
		{
			if (Action->Output.Exec)
				*Action->Output.Exec = BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL::Failed;

			Action->Output.ErrorMessage = Message;

			if (Action->Output.Condition.IsValid())
				*Action->Output.Condition.Get() = true;

			Action->ActionCompleted();
		});
	Action->OnRemoveTabbedWidgetSucceed.AddWeakLambda(Action, [Action]
	()
		{
			if (Action->Output.Exec)
				*Action->Output.Exec = BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL::Succeed;

			if (Action->Output.Condition.IsValid())
				*Action->Output.Condition.Get() = true;

			Action->ActionCompleted();
		});

	UBKManagerStore::GetActionManager()->QueueAction(Action);
}

void UBKActionRemoveTabbedWidget::Perform_RemoveTabbedWidget_Action_Cpp(
	const FBKActionRemoveTabbedWidgetInput& Input,
	const TFunction<void(const FString&)>& FailedCallback,
	const TFunction<void()>& SucceedCallback)
{
	UBKActionRemoveTabbedWidget* Action = NewObject<UBKActionRemoveTabbedWidget>(UBKManagerStore::GetActionManager());
	Action->Input = Input;
	Action->SetInputOutputPointers((FBKInputBase*)&Input, nullptr);

	Action->OnRemoveTabbedWidgetFailed.AddWeakLambda(Action, [Action, FailedCallback]
	(const FString& Message)
		{
			FailedCallback(Message);
			Action->ActionCompleted();
		});
	Action->OnRemoveTabbedWidgetSucceed.AddWeakLambda(Action, [Action, SucceedCallback]
	()
		{
			SucceedCallback();
			Action->ActionCompleted();
		});

	UBKManagerStore::GetActionManager()->QueueAction(Action);
}
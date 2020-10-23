/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "Actions/BActionCreateTabbedWidget.h"
#include "Managers/BManagerStore.h"
#include "Managers/BViewManager.h"
#include "Managers/BActionManager.h"

void UBKActionCreateTabbedWidget::Execute()
{
	Super::Execute();

	if (UBKViewManager* ViewManager = UBKManagerStore::GetViewManager())
	{
		TWeakObjectPtr<UBKActionCreateTabbedWidget> ThisPtr(this);
		ViewManager->CreateTabbedWidget(Output.OutUniqueId, Input.WidgetClass, Input.TabTitle, [ThisPtr]
			(bool bSuccess, const FString& ErrorMessage)
			{
				if (!ThisPtr.IsValid() || ThisPtr->IsActionCanceled()) return;

				if (bSuccess)
				{
					ThisPtr->OnCreateTabbedWidgetSucceed.Broadcast();
				}
				else
				{
					ThisPtr->OnCreateTabbedWidgetFailed.Broadcast(ErrorMessage);
				}
			});
	}
}

void UBKActionCreateTabbedWidget::Perform_CreateTabbedWidget_Action_BP(
	const FBKActionCreateTabbedWidgetInput& Input,
	BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
	FBKActionCreateTabbedWidgetOutput& Output,
	struct FLatentActionInfo LatentInfo)
{
	UBKActionCreateTabbedWidget* Action = NewObject<UBKActionCreateTabbedWidget>(UBKManagerStore::GetActionManager());
	Action->Input = Input;
	Action->Output.RelativeInput = Action->Input;
	Action->Output.InputPtr = &Action->Input;
	Action->SetInputOutputPointers((FBKInputBase*)&Input, (FBKOutputBase*)&Output);

	Action->CalledFromBP(Exec, LatentInfo, &Action->Output, &Output);

	Action->OnCreateTabbedWidgetFailed.AddWeakLambda(Action, [Action]
		(const FString& Message)
		{
			if (Action->Output.Exec)
				*Action->Output.Exec = BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL::Failed;

			Action->Output.ErrorMessage = Message;

			if (Action->Output.Condition.IsValid())
				*Action->Output.Condition.Get() = true;

			Action->ActionCompleted();
		});
	Action->OnCreateTabbedWidgetSucceed.AddWeakLambda(Action, [Action]
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

void UBKActionCreateTabbedWidget::Perform_CreateTabbedWidget_Action_Cpp(
	const FBKActionCreateTabbedWidgetInput& Input,
	const TFunction<void(const FString&)>& FailedCallback, 
	const TFunction<void()>& SucceedCallback)
{
	UBKActionCreateTabbedWidget* Action = NewObject<UBKActionCreateTabbedWidget>(UBKManagerStore::GetActionManager());
	Action->Input = Input;
	Action->SetInputOutputPointers((FBKInputBase*)&Input, nullptr);

	Action->OnCreateTabbedWidgetFailed.AddWeakLambda(Action, [Action, FailedCallback]
		(const FString& Message)
		{
			FailedCallback(Message);
			Action->ActionCompleted();
		});
	Action->OnCreateTabbedWidgetSucceed.AddWeakLambda(Action, [Action, SucceedCallback]
		()
		{
			SucceedCallback();
			Action->ActionCompleted();
		});

	UBKManagerStore::GetActionManager()->QueueAction(Action);
}
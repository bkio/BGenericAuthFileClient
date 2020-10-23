/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "Managers/BActionManager.h"
#include "Actions/BActionCreateModel.h"
#include "Actions/BActionLogin.h"
#include "Actions/BActionBase.h"

void UBKActionManager::OnConstruct()
{
	Super::OnConstruct();
}
void UBKActionManager::OnDestruct()
{
	//
	Super::OnDestruct();
}

void UBKActionManager::OnMapIsBeingTornDown()
{
}

void UBKActionManager::Initialize(bool _bExecuteOneActionPerTick)
{
	bExecuteOneActionPerTick = _bExecuteOneActionPerTick;
}

void UBKActionManager::QueueAction(UBKActionBase* Action)
{
	if (Action && Action->IsValidLowLevel() && !Action->IsPendingKillOrUnreachable())
		ActionsToExecute.Add(Action);
}

void UBKActionManager::Tick(float DeltaTime)
{
	if (bExecuteOneActionPerTick)
		ExecuteNextAction();
	else
		ExecuteAllActions();

	CleanUpFinishedActions();
}

void UBKActionManager::ExecuteAllActions()
{
	while (ActionsToExecute.Num() > 0)
		ExecuteNextAction();
}

void UBKActionManager::ExecuteNextAction()
{
	if (ActionsToExecute.Num() == 0) return;

	ActionsToExecute[0]->Execute();
	ActionsAwaitingResponse.Add(ActionsToExecute[0]);
	ActionsToExecute.RemoveAt(0);
}

void UBKActionManager::CleanUpFinishedActions()
{
	for (int32 i = ActionsAwaitingResponse.Num() - 1; i >= 0; i--)
	{
		if (ActionsAwaitingResponse[i]->IsActionCompleted() || ActionsAwaitingResponse[i]->IsActionCanceled())
		{
			if (ActionsAwaitingResponse[i]->DelayWaitedAfterActionCompleted == 2) //2 ticks needed
			{
				ActionsAwaitingResponse.RemoveAt(i);
			}
			else
			{
				ActionsAwaitingResponse[i]->DelayWaitedAfterActionCompleted++;
			}
		}
	}
}
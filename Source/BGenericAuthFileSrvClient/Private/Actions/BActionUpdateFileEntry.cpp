/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "Actions/BActionUpdateFileEntry.h"
#include "Managers/BManagerStore.h"
#include "Managers/BActionManager.h"
#include "Managers/BNetworkManager.h"
#include "Network/BHTTPWrapper.h"
#include "Engine/LatentActionManager.h"
#include "Network/BCancellationToken.h"
#include "JsonUtilities.h"

void UBKActionUpdateFileEntry::Execute()
{
	Super::Execute();

	FString Url = BASE_PATH;
	Url.Append("file/models/" + Input.ModelId + "/revisions/" + FString::FromInt(Input.RevisionIndex) + "/versions/" + FString::FromInt(Input.VersionIndex) + "/raw");

	TWeakObjectPtr<UBKActionUpdateFileEntry> ThisPtr(this);

	UBKManagerStore::GetNetworkManager()->Post(Url, JsonString,
		[ThisPtr]
		(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message, int32 ErrorCode)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled())
			{
				ThisPtr->OnUpdateFileEntryFailed.Broadcast(Message, ErrorCode);
			}
		},
		[ThisPtr]
		(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled())
			{
				ThisPtr->OnUpdateFileEntrySucceed.Broadcast(Message);
			}
		},
		Input.bUseCancellationToken,
		(Input.CancellationToken && Input.CancellationToken->IsValidLowLevel() && !Input.CancellationToken->IsPendingKillOrUnreachable()) ? Input.CancellationToken->GetData() : nullptr);
}

void UBKActionUpdateFileEntry::Perform_UpdateFileEntry_Action_BP(
	const FBKActionUpdateFileEntryInput& Input,
	BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
	FBKActionUpdateFileEntryOutput& Output,
	struct FLatentActionInfo LatentInfo)
{
	UBKActionUpdateFileEntry* Action = NewObject<UBKActionUpdateFileEntry>(UBKManagerStore::GetActionManager());

	FBKActionUpdateFileEntryInput NewInput;
	NewInput.ActionId = Input.ActionId;
	NewInput.bUseCancellationToken = Input.bUseCancellationToken;
	NewInput.CancellationToken = Input.CancellationToken;
	NewInput.FileEntry = Input.FileEntry;
	NewInput.ModelId = Input.ModelId;
	NewInput.RevisionIndex = Input.RevisionIndex;
	NewInput.VersionIndex = Input.VersionIndex;

	Action->Input = NewInput;
	Action->Output.RelativeInput = Action->Input;
	Action->Output.InputPtr = &Action->Input;
	Action->SetInputOutputPointers((FBKInputBase*)&Action->Input, (FBKOutputBase*)&Action->Output);

	Action->InitializeActionDefinition(FVersionFileEntry::StaticStruct(), Input.FileEntry)
		->ThisIsAnUpsertActionCustom(&Action->Input.FileEntry, 
			[Action](FBKDataStruct* InData) 
			{ 
				((FVersionFileEntry*)InData)->PruneForPostCall(Action->JsonObj); 
			})
		->CalledFromBP(Exec, LatentInfo, &Action->Output, &Output)
		->FinalizeActionDefinition();

	Action->OnUpdateFileEntryFailed.AddWeakLambda(Action, [Action]
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
	Action->OnUpdateFileEntrySucceed.AddWeakLambda(Action, [Action]
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

void UBKActionUpdateFileEntry::Perform_UpdateFileEntry_Action_Cpp(
	const FBKActionUpdateFileEntryInput& Input,
	const TFunction<void(const FString&, int32)>& FailureCallback,
	const TFunction<void(const FString&)>& SuccessCallback)
{
	UBKActionUpdateFileEntry* Action = NewObject<UBKActionUpdateFileEntry>(UBKManagerStore::GetActionManager());
	Action->Input = Input;
	Action->SetInputOutputPointers((FBKInputBase*)&Input, nullptr);

	Action->InitializeActionDefinition(FVersionFileEntry::StaticStruct(), Input.FileEntry)
		->ThisIsAnUpsertActionCustom(&Action->Input.FileEntry,
			[Action](FBKDataStruct* InData)
			{
				((FVersionFileEntry*)InData)->PruneForPostCall(Action->JsonObj);
			})
		->FinalizeActionDefinition();

	Action->OnUpdateFileEntryFailed.AddWeakLambda(Action, [Action, FailureCallback](const FString& Message, int32 ErrorCode)
		{
			FailureCallback(Message, ErrorCode);
			Action->ActionCompleted();
		});
	Action->OnUpdateFileEntrySucceed.AddWeakLambda(Action, [Action, SuccessCallback](const FString& Message)
		{
			SuccessCallback(Message);
			Action->ActionCompleted();
		});

	UBKManagerStore::GetActionManager()->QueueAction(Action);
}
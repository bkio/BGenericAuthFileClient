/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "Actions/BActionCreateVersion.h"
#include "Managers/BManagerStore.h"
#include "Managers/BActionManager.h"
#include "Managers/BNetworkManager.h"
#include "Network/BHTTPWrapper.h"
#include "Network/BCancellationToken.h"
#include "Engine/LatentActionManager.h"
#include "JsonUtilities.h"

void UBKActionCreateVersion::Execute()
{
	Super::Execute();

	FString Url = BASE_PATH;
	Url.Append("file/models/" + Input.ModelId + "/revisions/" + FString::FromInt(Input.RevisionIndex) + "/versions");

	TWeakObjectPtr<UBKActionCreateVersion> ThisPtr(this);

	UBKManagerStore::GetNetworkManager()->Put(Url, JsonString,
		[ThisPtr]
		(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message, int32 ErrorCode)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled())
			{
				ThisPtr->OnCreateVersionFailed.Broadcast(Message, ErrorCode);
			}
		},
		[ThisPtr]
		(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled() && HttpWrapper.IsValid())
			{
				HttpWrapper.Pin()->GetResponseObject(ThisPtr->JsonObj);
				ThisPtr->OnCreateVersionSucceed.Broadcast(ThisPtr->JsonObj->GetIntegerField("versionIndex"));
			}
		},
		Input.bUseCancellationToken,
		(Input.CancellationToken && Input.CancellationToken->IsValidLowLevel() && !Input.CancellationToken->IsPendingKillOrUnreachable()) ? Input.CancellationToken->GetData() : nullptr);
}

void UBKActionCreateVersion::Perform_CreateVersion_Action_BP(
	const FBKActionCreateVersionInput& Input,
	BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
	FBKActionCreateVersionOutput& Output,
	FLatentActionInfo LatentInfo)
{
	UBKActionCreateVersion* Action = NewObject<UBKActionCreateVersion>(UBKManagerStore::GetActionManager());
	Action->Input = Input;
	Action->Output.RelativeInput = Action->Input;
	Action->Output.InputPtr = &Action->Input;
	Action->SetInputOutputPointers((FBKInputBase*)&Input, (FBKOutputBase*)&Output);
	
	Action->InitializeActionDefinition(FModelRevisionVersion::StaticStruct(), Input.VersionInfo)
		->ThisIsAnUpdateAction(&Action->Input.VersionInfo)
		->CalledFromBP(Exec, LatentInfo, &Action->Output, &Output)
		->FinalizeActionDefinition();

	Action->OnCreateVersionFailed.AddWeakLambda(Action, [Action]
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
	Action->OnCreateVersionSucceed.AddWeakLambda(Action, [Action]
	(int32 VersionIndex)
		{
			if (Action->Output.Exec)
				*Action->Output.Exec = BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL::Succeed;

			Action->Output.VersionIndex = VersionIndex;

			if (Action->Output.Condition.IsValid())
				*Action->Output.Condition.Get() = true;

			Action->ActionCompleted();
		});

	UBKManagerStore::GetActionManager()->QueueAction(Action);
}

void UBKActionCreateVersion::Perform_CreateVersion_Action_Cpp(
	const FBKActionCreateVersionInput& Input,
	const TFunction<void(const FString&, int32)>& FailureCallback,
	const TFunction<void(int32)>& SuccessCallback)
{
	UBKActionCreateVersion* Action = NewObject<UBKActionCreateVersion>(UBKManagerStore::GetActionManager());
	Action->Input = Input;
	Action->SetInputOutputPointers((FBKInputBase*)&Input, nullptr);

	Action->InitializeActionDefinition(FModelRevisionVersion::StaticStruct(), Action->Input.VersionInfo)
		->ThisIsAnUpdateAction(&Action->Input.VersionInfo)
		->FinalizeActionDefinition();

	Action->OnCreateVersionFailed.AddWeakLambda(Action, [Action, FailureCallback](const FString& Message, int32 ErrorCode)
		{
			FailureCallback(Message, ErrorCode);
			Action->ActionCompleted();
		});
	Action->OnCreateVersionSucceed.AddWeakLambda(Action, [Action, SuccessCallback](int32 VersionIndex)
		{
			SuccessCallback(VersionIndex);
			Action->ActionCompleted();
		});

	UBKManagerStore::GetActionManager()->QueueAction(Action);
}
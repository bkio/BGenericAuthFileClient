/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "Actions/BActionGetMetadataNode.h"
#include "Managers/BManagerStore.h"
#include "Managers/BActionManager.h"
#include "Managers/BNetworkManager.h"
#include "Network/BHTTPWrapper.h"
#include "Network/BCancellationToken.h"
#include "Engine/LatentActionManager.h"
#include "JsonUtilities.h"

void UBKActionGetMetadataNode::Execute()
{
	Super::Execute();

	FString Url = BASE_PATH;
	Url.Append("file/models/" + Input.ModelId + "/revisions/" + FString::FromInt(Input.RevisionIndex) + "/versions/" + FString::FromInt(Input.VersionIndex) + "/metadata/nodes/" + ConvertUInt64ToFString((uint64)Input.NodeID));

	TWeakObjectPtr<UBKActionGetMetadataNode> ThisPtr(this);

	UBKManagerStore::GetNetworkManager()->Get(Url,
		[ThisPtr]
		(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message, int32 ErrorCode)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled())
			{
				ThisPtr->OnGetMetadataNodeFailed.Broadcast(Message, ErrorCode);
			}
		},
		[ThisPtr]
		(TWeakPtr<BKHTTPWrapper> HttpWrapper, const FString& Message)
		{
			if (ThisPtr.IsValid() && !ThisPtr->IsActionCanceled() && HttpWrapper.IsValid())
			{
				FJMetadataNode Node;
				HttpWrapper.Pin()->GetResponseObject(ThisPtr->JsonObj);
				FJsonObjectConverter::JsonObjectStringToUStruct<FJMetadataNode>(Message, &Node, 0, 0);

				ThisPtr->OnGetMetadataNodeSucceed.Broadcast(Node);
			}
		},
		Input.bUseCancellationToken,
		(Input.CancellationToken && Input.CancellationToken->IsValidLowLevel() && !Input.CancellationToken->IsPendingKillOrUnreachable()) ? Input.CancellationToken->GetData() : nullptr);
}

void UBKActionGetMetadataNode::Perform_GetMetadataNode_Action_BP(
	const FBKActionGetMetadataNodeInput& Input,
	BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
	FBKActionGetMetadataNodeOutput& Output,
	struct FLatentActionInfo LatentInfo)
{
	UBKActionGetMetadataNode* Action = NewObject<UBKActionGetMetadataNode>();
	Action->Input = Input;
	Action->Output.RelativeInput = Action->Input;
	Action->Output.InputPtr = &Action->Input;
	Action->SetInputOutputPointers((FBKInputBase*)&Input, (FBKOutputBase*)&Output);

	Action->CalledFromBP(Exec, LatentInfo, &Action->Output, &Output);

	Action->OnGetMetadataNodeFailed.AddWeakLambda(Action, [Action]
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
	Action->OnGetMetadataNodeSucceed.AddWeakLambda(Action, [Action]
		(const FJMetadataNode& Node)
		{
			if (Action->Output.Exec)
				*Action->Output.Exec = BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL::Succeed;

			Action->Output.Node = Node;

			if (Action->Output.Condition.IsValid())
				*Action->Output.Condition.Get() = true;

			Action->ActionCompleted();
		});

	UBKManagerStore::GetActionManager()->QueueAction(Action);
}

void UBKActionGetMetadataNode::Perform_GetMetadataNode_Action_Cpp(
	const FBKActionGetMetadataNodeInput& Input,
	const TFunction<void(const FString&, int32)>& FailureCallback,
	const TFunction<void(const FJMetadataNode&)>& SuccessCallback)
{
	UBKActionGetMetadataNode* Action = NewObject<UBKActionGetMetadataNode>();
	Action->Input = Input;
	Action->SetInputOutputPointers((FBKInputBase*)&Input, nullptr);

	Action->OnGetMetadataNodeFailed.AddWeakLambda(Action, [Action, FailureCallback]
		(const FString& Message, int32 ErrorCode)
		{
			FailureCallback(Message, ErrorCode);
			Action->ActionCompleted();
		});
	Action->OnGetMetadataNodeSucceed.AddWeakLambda(Action, [Action, SuccessCallback]
		(const FJMetadataNode& Node)
		{
			SuccessCallback(Node);
			Action->ActionCompleted();
		});

	UBKManagerStore::GetActionManager()->QueueAction(Action);
}
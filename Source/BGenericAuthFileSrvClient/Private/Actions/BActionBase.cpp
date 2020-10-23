/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "Actions/BActionBase.h"
#include "Managers/BActionManager.h"
#include "Network/BCancellationToken.h"
#include "UObject/Class.h"
#include "JsonUtilities.h"
#include "JsonObjectConverter.h"
#include "Engine/LatentActionManager.h"
#include "Engine/World.h"
#include <sstream>

bool UBKActionBase::IsActionCompleted() const
{
	return bActionCompleted;
}

bool UBKActionBase::IsActionCanceled()
{
	if (!bActionCanceled)
	{
		bActionCanceled = (InputPtr && InputPtr->bUseCancellationToken && (
			!InputPtr->CancellationToken
			|| !InputPtr->CancellationToken->IsValidLowLevel()
			|| InputPtr->CancellationToken->IsPendingKillOrUnreachable()
			|| !InputPtr->CancellationToken->IsTokenValid()));
		if (bActionCanceled)
		{
			ActionCanceled();
		}
	}
	return bActionCanceled;
}

void UBKActionBase::ActionCompleted()
{
	bActionCompleted = true;
	OnActionCompleted.Broadcast();
}

void UBKActionBase::ActionCanceled()
{
	bActionCanceled = true;
	OnActionCanceled.Broadcast();
}

void UBKActionBase::SetInputOutputPointers(FBKInputBase* InInputPtr, FBKOutputBase* InOutputPtr)
{
	InputPtr = InInputPtr;
	OutputPtr = InOutputPtr;
}

#define ACTION_ID_LENGTH 32
FString UBKActionBase::GenerateActionId()
{
	return UBKActionManager::GenerateRandomANSIString().Mid(0, ACTION_ID_LENGTH);
}

UBKActionBase* UBKActionBase::InitializeActionDefinition(UScriptStruct* DataStaticStruct, const FBKDataStruct& Data)
{
	FJsonObjectConverter::UStructToJsonObjectString(DataStaticStruct, &Data, JsonString, 0, 0);

	//Decompile to FJsonObject
	JsonObj = MakeShareable(new FJsonObject());
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);

	if (!FJsonSerializer::Deserialize(JsonReader, JsonObj) || !JsonObj.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("Deserializing file data failed. Data: %s."), *JsonString);
		return this;
	}
	return this;
}

UBKActionBase* UBKActionBase::ThisIsAnUpdateAction(FBKDataStruct* Data)
{
	Data->PruneForUpdateCall(JsonObj);
	return this;
}

UBKActionBase* UBKActionBase::ThisIsAnUpsertActionCustom(FBKDataStruct* Data, TFunction<void(FBKDataStruct*)> CallThis)
{
	CallThis(Data);
	return this;
}

UBKActionBase* UBKActionBase::CalledFromBP(BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec, FLatentActionInfo& LatentInfo, FBKOutputBase* ActionOutput, FBKOutputBase* FunctionOutput)
{
	check (ActionOutput->InputPtr && ActionOutput->InputPtr->ActionId.Len() == ACTION_ID_LENGTH);
	ActionOutput->Exec = &Exec;
	PrepareLatentBPExecAction(LatentInfo, ActionOutput, FunctionOutput);
	return this;
}

UBKActionBase* UBKActionBase::FinalizeActionDefinition()
{
	//Compile to FString
	auto Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonObj.ToSharedRef(), Writer);
	return this;
}

void UBKActionBase::Execute()
{
	TWeakObjectPtr<UBKActionBase> ThisPtr(this);
	OnActionCanceled.AddWeakLambda(this,
		[ThisPtr]
		()
		{
			if (!ThisPtr.IsValid()) return;
			if (!ThisPtr->OutputPtr) return;

			if (ThisPtr->OutputPtr->Exec)
			{
				*ThisPtr->OutputPtr->Exec = BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL::Canceled;
			}

			if (ThisPtr->OutputPtr->Condition.IsValid())
				*ThisPtr->OutputPtr->Condition.Get() = true;
		});
}

EBKActionErrorCode UBKActionBase::ConvertHttpCodeToActionErrorCode(int32 ErrorCode)
{
	switch (ErrorCode)
	{
		case 400:
			return EBKActionErrorCode::BadRequest;
		case 401:
		case 403:
			return EBKActionErrorCode::LoginRequired;
		case 404:
			return EBKActionErrorCode::NotFound;
		case 409:
			return EBKActionErrorCode::Conflict;
		case 415:
			return EBKActionErrorCode::UnsupportedFileType;
		default:
			return EBKActionErrorCode::InternalError;
	}
}

FString UBKActionBase::ConvertUInt64ToFString(uint64 Numeric)
{
	std::ostringstream oss;
	oss << Numeric;
	return FString(oss.str().c_str());
}

FString UBKActionBase::ConvertInt64ToFString(int64 Numeric)
{
	std::ostringstream oss;
	oss << Numeric;
	return FString(oss.str().c_str());
}

void UBKActionBase::PrepareLatentBPExecAction(FLatentActionInfo& LatentInfo, FBKOutputBase* ActionSpecificOutputPtr, FBKOutputBase* LatentFunctionOutputPtr)
{
	check (ActionSpecificOutputPtr != LatentFunctionOutputPtr);

	if (UWorld* World = GEditor->GetEditorWorldContext().World())
	{
		if (World->IsValidLowLevel() && !World->IsPendingKillOrUnreachable())
		{
			FLatentActionManager& LatentActionManager = World->GetLatentActionManager();

			if (FBKLatentAction_Internal* ExistingAction = LatentActionManager.FindExistingAction<FBKLatentAction_Internal>(LatentInfo.CallbackTarget, LatentInfo.UUID))
			{
				ExistingAction->InitializeOrAdd(
					LatentInfo,
					ActionSpecificOutputPtr,
					LatentFunctionOutputPtr);
			}
			else
			{
				LatentActionManager.AddNewAction(
					LatentInfo.CallbackTarget,
					LatentInfo.UUID,
					new FBKLatentAction_Internal(
						LatentInfo,
						ActionSpecificOutputPtr,
						LatentFunctionOutputPtr));
			}
		}
	}
}

void FBKLatentAction_Internal::UpdateOperation(FLatentResponse& Response)
{
	ActionCurrentTickNo++;
	UpdateOperation_Internal(Response);
}
void FBKLatentAction_Internal::UpdateOperation_Internal(FLatentResponse& Response)
{
	if (ActionOutputs.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("UpdateOperation_Internal: ActionOutputs does not have any elements. Cancelling the action."));
		Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
		return;
	}

	FBKOutputBase* OnProgressTickAction = nullptr;

	for (int32 i = ActionOutputs.Num() - 1; i >= 0; i--)
	{
		if (FBKOutputBase* ActionOutput = ActionOutputs[i])
		{
			if (ActionOutput->Condition.IsValid() && *ActionOutput->Condition.Get() == true)
			{
				ActionOutputs.RemoveAt(i);
				FunctionOutputPtr->SetProperties(ActionOutput);

				if (ActionOutputs.Num() == 0)
				{
					Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
				}
				else
				{
					Response.TriggerLink(ExecutionFunction, OutputLink, CallbackTarget);
				}
				return; //One exec at a tick.
			}

			if (OnProgressTickAction == nullptr
				|| ActionOutput->LastOnProgressReceivedTickNo < OnProgressTickAction->LastOnProgressReceivedTickNo)
			{
				OnProgressTickAction = ActionOutput;
			}
		}
		else
		{
			ActionOutputs.RemoveAt(i);
		}
	}

	if (OnProgressTickAction != nullptr)
	{
		OnProgressTickAction->LastOnProgressReceivedTickNo = ActionCurrentTickNo;
		FunctionOutputPtr->SetProperties(OnProgressTickAction);
		*FunctionOutputPtr->Exec = BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL::OnProgress;
		Response.TriggerLink(ExecutionFunction, OutputLink, CallbackTarget);
	}
}

void FBKLatentAction_Internal::InitializeOrAdd(const FLatentActionInfo& LatentInfo, FBKOutputBase* ActionSpecificOutputParameter, FBKOutputBase* FunctionOutputParameter)
{
	ActionOutputs.AddUnique(ActionSpecificOutputParameter);
	if (bFirstSet)
	{
		bFirstSet = false;
		ExecutionFunction = LatentInfo.ExecutionFunction;
		OutputLink = LatentInfo.Linkage;
		CallbackTarget = LatentInfo.CallbackTarget;
		FunctionOutputPtr = FunctionOutputParameter;
	}
	else
	{
		check(ExecutionFunction == LatentInfo.ExecutionFunction);
		check(OutputLink == LatentInfo.Linkage);
		check(CallbackTarget == LatentInfo.CallbackTarget);
		check(FunctionOutputPtr == FunctionOutputParameter);
	}
}
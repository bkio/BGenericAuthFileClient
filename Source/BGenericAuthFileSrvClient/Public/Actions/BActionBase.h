/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "CoreMinimal.h"
#include "Data/BData.h"
#include "UObject/NoExportTypes.h"
#include "Runtime/Engine/Public/LatentActions.h"
#include "BActionBase.generated.h"

UENUM(BlueprintType)
enum class BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL : uint8
{
	OnProgress = 0,
	Succeed = 1,
	Failed = 2,
	Canceled = 3
};

UENUM(BlueprintType)
enum EBKActionErrorCode
{
	BadRequest = 0,
	LoginRequired = 1,
	NotFound = 2,
	Conflict = 3,
	UnsupportedFileType = 4,
	InternalError = 5
};

USTRUCT(BlueprintType)
struct FBKInputBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FString ActionId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	bool bUseCancellationToken = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	class UBKCancellationToken* CancellationToken = nullptr;

};

USTRUCT(BlueprintType)
struct FBKOutputBase
{
	GENERATED_USTRUCT_BODY()

public:
	TSharedPtr<bool> Condition;
	BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL* Exec;

	uint32 LastOnProgressReceivedTickNo = 0;

	FBKInputBase* InputPtr;

	FBKOutputBase()
	{
		Condition = MakeShareable(new bool(false));
	}

	virtual void SetProperties(FBKOutputBase* Other) {}
	virtual ~FBKOutputBase() {}
};

class FBKLatentAction_Internal : public FPendingLatentAction
{
private:
	bool bFirstSet = true;
	bool bDoneCall = false;

	uint32 ActionCurrentTickNo = 0;

	void UpdateOperation_Internal(FLatentResponse& Response);

public:
	FName ExecutionFunction;
	int32 OutputLink;
	FWeakObjectPtr CallbackTarget;

	TArray<FBKOutputBase*> ActionOutputs;
	FBKOutputBase* FunctionOutputPtr;

	void InitializeOrAdd(const FLatentActionInfo& LatentInfo, FBKOutputBase* ActionSpecificOutputParameter, FBKOutputBase* FunctionOutputParameter);

	FBKLatentAction_Internal(const FLatentActionInfo& LatentInfo, FBKOutputBase* ActionSpecificOutputParameter, FBKOutputBase* FunctionOutputParameter)
	{
		InitializeOrAdd(LatentInfo, ActionSpecificOutputParameter, FunctionOutputParameter);
	}

	virtual void UpdateOperation(FLatentResponse& Response) override;
};

UCLASS()
class BGENERICAUTHFILESRVCLIENT_API UBKActionBase : public UObject
{
	GENERATED_BODY()
	
public:
	virtual void Execute();

	bool IsActionCompleted() const;
	bool IsActionCanceled();

	DECLARE_MULTICAST_DELEGATE(FActionCompleted);
	FActionCompleted OnActionCompleted;

	DECLARE_MULTICAST_DELEGATE(FActionCanceled);
	FActionCanceled OnActionCanceled;

	int32 DelayWaitedAfterActionCompleted = 0; //This is for the sake of BP latent actions. Called and controlled by action manager

	UBKActionBase* InitializeActionDefinition(class UScriptStruct* DataStaticStruct, const FBKDataStruct& Data);
	UBKActionBase* ThisIsAnUpdateAction(FBKDataStruct* Data);
	UBKActionBase* ThisIsAnUpsertActionCustom(FBKDataStruct* Data, TFunction<void(FBKDataStruct*)> CallThis);
	UBKActionBase* FinalizeActionDefinition();
	UBKActionBase* CalledFromBP(BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec, FLatentActionInfo& LatentInfo, FBKOutputBase* ActionOutput, FBKOutputBase* FunctionOutput);

	UFUNCTION(BlueprintCallable, Category = "BGenericAuthFileSrvClient|Actions")
	static FString GenerateActionId();

protected:
	TSharedPtr<class FJsonObject> JsonObj;
	FString JsonString;

	static EBKActionErrorCode ConvertHttpCodeToActionErrorCode(int32 ErrorCode);

	static FString ConvertUInt64ToFString(uint64 Numeric);
	static FString ConvertInt64ToFString(int64 Numeric);

	void ActionCompleted();
	void ActionCanceled();

	void SetInputOutputPointers(FBKInputBase* InInputPtr, FBKOutputBase* InOutputPtr);

	friend class UBKActionBlueprintFunctionLibrary;

private:
	bool bActionCompleted = false;
	bool bActionCanceled = false;

	FBKInputBase* InputPtr;
	FBKOutputBase* OutputPtr;

	static void PrepareLatentBPExecAction(struct FLatentActionInfo& LatentInfo, FBKOutputBase* ActionSpecificOutputPtr, FBKOutputBase* LatentFunctionOutputPtr);
};
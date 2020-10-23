/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "CoreMinimal.h"
#include "Actions/BActionBase.h"
#include "Data/BProcessData.h"
#include "BActionGetGeometryNode.generated.h"

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FBKActionGetGeometryNodeInput : public FBKInputBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FString ModelId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	int32 RevisionIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	int32 VersionIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	int64 NodeID;
};

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FBKActionGetGeometryNodeOutput : public FBKOutputBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FBKActionGetGeometryNodeInput RelativeInput;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FJGeometryNode Node;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FString ErrorMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	TEnumAsByte<EBKActionErrorCode> ErrorCode;

	virtual void SetProperties(FBKOutputBase* Other) override
	{
		FBKActionGetGeometryNodeOutput* OtherCasted = (FBKActionGetGeometryNodeOutput*)Other;
		*this = *OtherCasted;
	}
};

UCLASS()
class UBKActionGetGeometryNode : public UBKActionBase
{
	GENERATED_BODY()

public:
	/*
	* For blueprint usage
	*/
	UFUNCTION(BlueprintCallable, Category = "BGenericAuthFileSrvClient|Actions", DisplayName = "Perform GetGeometryNode Action", meta = (ExpandEnumAsExecs = "Exec", Latent, LatentInfo = "LatentInfo"))
	static void Perform_GetGeometryNode_Action_BP(
		const FBKActionGetGeometryNodeInput& Input,
		BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
		FBKActionGetGeometryNodeOutput& Output,
		struct FLatentActionInfo LatentInfo);

	/*
	* For all in one cpp usage
	*/
	static void Perform_GetGeometryNode_Action_Cpp(
		const FBKActionGetGeometryNodeInput& Input,
		const TFunction<void(const FString&, int32)>& FailureCallback,
		const TFunction<void(const FJGeometryNode&)>& SuccessCallback);

	/*
	* For custom cpp use cases that require multiple listeners binded: NewObject<>(); then bind delegates; then ActionManager->QueueAction.
	*/
	DECLARE_MULTICAST_DELEGATE_TwoParams(FGetGeometryNodeFailed, const FString&, int32);
	FGetGeometryNodeFailed OnGetGeometryNodeFailed;

	DECLARE_MULTICAST_DELEGATE_OneParam(FGetGeometryNodeSucceed, const FJGeometryNode&);
	FGetGeometryNodeSucceed OnGetGeometryNodeSucceed;

private:
	UPROPERTY()
	FBKActionGetGeometryNodeInput Input;
	UPROPERTY()
	FBKActionGetGeometryNodeOutput Output;

	virtual void Execute() override;
};
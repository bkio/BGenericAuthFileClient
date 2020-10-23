/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "CoreMinimal.h"
#include "Actions/BActionBase.h"
#include "Data/BProcessData.h"
#include "BActionGetHierarchyNode.generated.h"

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FBKActionGetHierarchyNodeInput : public FBKInputBase
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
struct BGENERICAUTHFILESRVCLIENT_API FBKActionGetHierarchyNodeOutput : public FBKOutputBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FBKActionGetHierarchyNodeInput RelativeInput;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FJHierarchyNode Node;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FString ErrorMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	TEnumAsByte<EBKActionErrorCode> ErrorCode;

	virtual void SetProperties(FBKOutputBase* Other) override
	{
		FBKActionGetHierarchyNodeOutput* OtherCasted = (FBKActionGetHierarchyNodeOutput*)Other;
		*this = *OtherCasted;
	}
};

UCLASS()
class UBKActionGetHierarchyNode : public UBKActionBase
{
	GENERATED_BODY()

public:
	/*
	* For blueprint usage
	*/
	UFUNCTION(BlueprintCallable, Category = "BGenericAuthFileSrvClient|Actions", DisplayName = "Perform GetHierarchyNode Action", meta = (ExpandEnumAsExecs = "Exec", Latent, LatentInfo = "LatentInfo"))
	static void Perform_GetHierarchyNode_Action_BP(
		const FBKActionGetHierarchyNodeInput& Input,
		BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
		FBKActionGetHierarchyNodeOutput& Output,
		struct FLatentActionInfo LatentInfo);

	/*
	* For all in one cpp usage
	*/
	static void Perform_GetHierarchyNode_Action_Cpp(
		const FBKActionGetHierarchyNodeInput& Input,
		const TFunction<void(const FString&, int32)>& FailureCallback,
		const TFunction<void(const FJHierarchyNode&)>& SuccessCallback);

	/*
	* For custom cpp use cases that require multiple listeners binded: NewObject<>(); then bind delegates; then ActionManager->QueueAction.
	*/
	DECLARE_MULTICAST_DELEGATE_TwoParams(FGetHierarchyNodeFailed, const FString&, int32);
	FGetHierarchyNodeFailed OnGetHierarchyNodeFailed;

	DECLARE_MULTICAST_DELEGATE_OneParam(FGetHierarchyNodeSucceed, const FJHierarchyNode&);
	FGetHierarchyNodeSucceed OnGetHierarchyNodeSucceed;

private:
	UPROPERTY()
	FBKActionGetHierarchyNodeInput Input;
	UPROPERTY()
	FBKActionGetHierarchyNodeOutput Output;

	virtual void Execute() override;
};
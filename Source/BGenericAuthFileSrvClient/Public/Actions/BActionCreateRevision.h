/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "CoreMinimal.h"
#include "Actions/BActionBase.h"
#include "Data/BModelData.h"
#include "BActionCreateRevision.generated.h"

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FBKActionCreateRevisionInput : public FBKInputBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FString ModelId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FModelRevision RevisionInfo;
};

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FBKActionCreateRevisionOutput : public FBKOutputBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FBKActionCreateRevisionInput RelativeInput;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	int32 RevisionIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FString ErrorMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	TEnumAsByte<EBKActionErrorCode> ErrorCode;

	virtual void SetProperties(FBKOutputBase* Other) override
	{
		FBKActionCreateRevisionOutput* OtherCasted = (FBKActionCreateRevisionOutput*)Other;
		*this = *OtherCasted;
	}
};

UCLASS()
class BGENERICAUTHFILESRVCLIENT_API UBKActionCreateRevision : public UBKActionBase
{
	GENERATED_BODY()

public:
	/*
	* For blueprint usage
	*/
	UFUNCTION(BlueprintCallable, Category = "BGenericAuthFileSrvClient|Actions", DisplayName = "Perform CreateRevision Action", meta = (ExpandEnumAsExecs = "Exec", Latent, LatentInfo = "LatentInfo"))
	static void Perform_CreateRevision_Action_BP(
		const FBKActionCreateRevisionInput& Input,
		BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
		FBKActionCreateRevisionOutput& Output,
		struct FLatentActionInfo LatentInfo);

	/*
	* For all in one cpp usage
	*/
	static void Perform_CreateRevision_Action_Cpp(
		const FBKActionCreateRevisionInput& Input,
		const TFunction<void(const FString&, int32)>& FailureCallback,
		const TFunction<void(int32)>& SuccessCallback);

	/*
	* For custom cpp use cases that require multiple listeners binded.
	* NewObject<>(); then bind delegates;
	* then InitializeActionDefinition->ThisIsAnUpdateAction->FinalizeActionDefinition
	* then ActionManager->QueueAction.
	*/
	DECLARE_MULTICAST_DELEGATE_TwoParams(FCreateRevisionFailed, const FString&, int32);
	FCreateRevisionFailed OnCreateRevisionFailed;

	DECLARE_MULTICAST_DELEGATE_OneParam(FCreateRevisionSucceed, const int32);
	FCreateRevisionSucceed OnCreateRevisionSucceed;

private:
	UPROPERTY()
	FBKActionCreateRevisionInput Input;
	UPROPERTY()
	FBKActionCreateRevisionOutput Output;

	virtual void Execute() override;
};
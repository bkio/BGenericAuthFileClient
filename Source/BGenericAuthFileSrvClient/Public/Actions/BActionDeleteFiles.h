/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "CoreMinimal.h"
#include "Actions/BActionBase.h"
#include "Data/BModelData.h"
#include "BActionDeleteFiles.generated.h"

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FBKActionDeleteFilesInput : public FBKInputBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FString ModelId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	int32 RevisionIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	int32 VersionIndex;
};

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FBKActionDeleteFilesOutput : public FBKOutputBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FBKActionDeleteFilesInput RelativeInput;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FString Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	TEnumAsByte<EBKActionErrorCode> ErrorCode;

	virtual void SetProperties(FBKOutputBase* Other) override
	{
		FBKActionDeleteFilesOutput* OtherCasted = (FBKActionDeleteFilesOutput*)Other;
		*this = *OtherCasted;
	}
};

UCLASS()
class BGENERICAUTHFILESRVCLIENT_API UBKActionDeleteFiles : public UBKActionBase
{
	GENERATED_BODY()

public:
	/*
	* For blueprint usage
	*/
	UFUNCTION(BlueprintCallable, Category = "BGenericAuthFileSrvClient|Actions", DisplayName = "Perform DeleteFiles Action", meta = (ExpandEnumAsExecs = "Exec", Latent, LatentInfo = "LatentInfo"))
	static void Perform_DeleteFiles_Action_BP(
		const FBKActionDeleteFilesInput& Input,
		BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
		FBKActionDeleteFilesOutput& Output,
		struct FLatentActionInfo LatentInfo);

	/*
	* For all in one cpp usage
	*/
	static void Perform_DeleteFiles_Action_Cpp(
		const FBKActionDeleteFilesInput& Input,
		const TFunction<void(const FString&, int32)>& FailureCallback,
		const TFunction<void(const FString&)>& SuccessCallback);

	/*
	* For custom cpp use cases that require multiple listeners binded.
	* NewObject<>(); then bind delegates;
	* then ActionManager->QueueAction.
	*/
	DECLARE_MULTICAST_DELEGATE_TwoParams(FDeleteFilesFailed, const FString&, int32);
	FDeleteFilesFailed OnDeleteFilesFailed;

	DECLARE_MULTICAST_DELEGATE_OneParam(FDeleteFilesSucceed, const FString&);
	FDeleteFilesSucceed OnDeleteFilesSucceed;

private:
	UPROPERTY()
	FBKActionDeleteFilesInput Input;
	UPROPERTY()
	FBKActionDeleteFilesOutput Output;

	virtual void Execute() override;
};
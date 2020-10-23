/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "CoreMinimal.h"
#include "Actions/BActionBase.h"
#include "Data/BModelData.h"
#include "BActionDownloadRawFile.generated.h"

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FBKActionDownloadRawFileInput : public FBKInputBase
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
	FString DestinationLocalPath;
};

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FBKActionDownloadRawFileOutput : public FBKOutputBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FBKActionDownloadRawFileInput RelativeInput;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	float Progress = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FString Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	TEnumAsByte<EBKActionErrorCode> ErrorCode;

	virtual void SetProperties(FBKOutputBase* Other) override
	{
		FBKActionDownloadRawFileOutput* OtherCasted = (FBKActionDownloadRawFileOutput*)Other;
		*this = *OtherCasted;
	}
};

UCLASS()
class BGENERICAUTHFILESRVCLIENT_API UBKActionDownloadRawFile : public UBKActionBase
{
	GENERATED_BODY()

public:
	/*
	* For blueprint usage
	*/
	UFUNCTION(BlueprintCallable, Category = "BGenericAuthFileSrvClient|Actions", DisplayName = "Perform DownloadRawFile Action", meta = (ExpandEnumAsExecs = "Exec", Latent, LatentInfo = "LatentInfo"))
	static void Perform_DownloadRawFile_Action_BP(
		const FBKActionDownloadRawFileInput& Input,
		BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
		FBKActionDownloadRawFileOutput& Output,
		struct FLatentActionInfo LatentInfo);

	/*
	* For all in one cpp usage
	*/
	static void Perform_DownloadRawFile_Action_Cpp(
		const FBKActionDownloadRawFileInput& Input,
		const TFunction<void(const FString&, int32)>& FailureCallback,
		const TFunction<void(float)>& OnProgressCallback,
		const TFunction<void(const FString&)>& SuccessCallback);

	/*
	* For custom cpp use cases that require multiple listeners binded.
	* NewObject<>(); then bind delegates;
	* then ActionManager->QueueAction.
	*/
	DECLARE_MULTICAST_DELEGATE_TwoParams(FDownloadRawFileFailed, const FString&, int32);
	FDownloadRawFileFailed OnDownloadRawFileFailed;

	DECLARE_MULTICAST_DELEGATE_OneParam(FDownloadRawFileSucceed, const FString&);
	FDownloadRawFileSucceed OnDownloadRawFileSucceed;

	DECLARE_MULTICAST_DELEGATE_OneParam(FDownloadRawFileProgress, float);
	FDownloadRawFileProgress OnDownloadRawFileProgress;

private:
	UPROPERTY()
	FBKActionDownloadRawFileInput Input;
	UPROPERTY()
	FBKActionDownloadRawFileOutput Output;

	virtual void Execute() override;
};
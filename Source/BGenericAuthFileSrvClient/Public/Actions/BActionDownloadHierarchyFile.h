/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "CoreMinimal.h"
#include "Actions/BActionBase.h"
#include "Data/BModelData.h"
#include "BActionDownloadHierarchyFile.generated.h"

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FBKActionDownloadHierarchyFileInput : public FBKInputBase
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
struct BGENERICAUTHFILESRVCLIENT_API FBKActionDownloadHierarchyFileOutput : public FBKOutputBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FBKActionDownloadHierarchyFileInput RelativeInput;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	float Progress;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FString Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	TEnumAsByte<EBKActionErrorCode> ErrorCode;

	virtual void SetProperties(FBKOutputBase* Other) override
	{
		FBKActionDownloadHierarchyFileOutput* OtherCasted = (FBKActionDownloadHierarchyFileOutput*)Other;
		*this = *OtherCasted;
	}
};

UCLASS()
class BGENERICAUTHFILESRVCLIENT_API UBKActionDownloadHierarchyFile : public UBKActionBase
{
	GENERATED_BODY()

public:
	/*
	* For blueprint usage
	*/
	UFUNCTION(BlueprintCallable, Category = "BGenericAuthFileSrvClient|Actions", DisplayName = "Perform DownloadHierarchyFile Action", meta = (ExpandEnumAsExecs = "Exec", Latent, LatentInfo = "LatentInfo"))
	static void Perform_DownloadHierarchyFile_Action_BP(
		const FBKActionDownloadHierarchyFileInput& Input,
		BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
		FBKActionDownloadHierarchyFileOutput& Output,
		struct FLatentActionInfo LatentInfo);

	/*
	* For all in one cpp usage
	*/
	static void Perform_DownloadHierarchyFile_Action_Cpp(
		const FBKActionDownloadHierarchyFileInput& Input,
		const TFunction<void(const FString&, int32)>& FailureCallback,
		const TFunction<void(float)>& OnProgressCallback,
		const TFunction<void(const FString&)>& SuccessCallback);

	/*
	* For custom cpp use cases that require multiple listeners binded.
	* NewObject<>(); then bind delegates;
	* then ActionManager->QueueAction.
	*/
	DECLARE_MULTICAST_DELEGATE_TwoParams(FDownloadHierarchyFileFailed, const FString&, int32);
	FDownloadHierarchyFileFailed OnDownloadHierarchyFileFailed;

	DECLARE_MULTICAST_DELEGATE_OneParam(FDownloadHierarchyFileSucceed, const FString&);
	FDownloadHierarchyFileSucceed OnDownloadHierarchyFileSucceed;

	DECLARE_MULTICAST_DELEGATE_OneParam(FDownloadHierarchyFileProgress, float);
	FDownloadHierarchyFileProgress OnDownloadHierarchyFileProgress;

private:
	UPROPERTY()
	FBKActionDownloadHierarchyFileInput Input;
	UPROPERTY()
	FBKActionDownloadHierarchyFileOutput Output;

	virtual void Execute() override;
};
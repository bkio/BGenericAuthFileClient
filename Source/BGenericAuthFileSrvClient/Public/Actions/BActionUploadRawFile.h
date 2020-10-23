/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "CoreMinimal.h"
#include "Actions/BActionBase.h"
#include "Data/BModelData.h"
#include <fstream>
#include "BActionUploadRawFile.generated.h"

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FBKActionUploadRawFileInput : public FBKInputBase
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
	FString FileEntryName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	TArray<FString> FilePaths;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FString TopAssemblyNameIfAny;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	TArray<FString> PotentialCADSoftwareNames;
};

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FBKActionUploadRawFileOutput : public FBKOutputBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FBKActionUploadRawFileInput RelativeInput;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	float Progress;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FString Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	TEnumAsByte<EBKActionErrorCode> ErrorCode;

	virtual void SetProperties(FBKOutputBase* Other) override
	{
		FBKActionUploadRawFileOutput* OtherCasted = (FBKActionUploadRawFileOutput*)Other;
		*this = *OtherCasted;
	}
};

UCLASS()
class BGENERICAUTHFILESRVCLIENT_API UBKActionUploadRawFile : public UBKActionBase
{
	GENERATED_BODY()

public:
	/*
	* For blueprint usage
	*/
	UFUNCTION(BlueprintCallable, Category = "BGenericAuthFileSrvClient|Actions", DisplayName = "Perform UploadRawFile Action", meta = (ExpandEnumAsExecs = "Exec", Latent, LatentInfo = "LatentInfo"))
	static void Perform_UploadRawFile_Action_BP(
		const FBKActionUploadRawFileInput& Input,
		BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
		FBKActionUploadRawFileOutput& Output,
		struct FLatentActionInfo LatentInfo);

	/*
	* For all in one cpp usage
	*/
	static void Perform_UploadRawFile_Action_Cpp(
		const FBKActionUploadRawFileInput& Input,
		const TFunction<void(const FString&, int32)>& FailureCallback,
		const TFunction<void(float)>& OnProgressCallback,
		const TFunction<void(const FString&)>& SuccessCallback);

	/*
	* For custom cpp use cases that require multiple listeners binded.
	* NewObject<>(); then bind delegates;
	* then ActionManager->QueueAction.
	*/
	DECLARE_MULTICAST_DELEGATE_TwoParams(FUploadRawFileFailed, const FString&, int32);
	FUploadRawFileFailed OnUploadRawFileFailed;

	DECLARE_MULTICAST_DELEGATE_OneParam(FUploadRawFileSucceed, const FString&);
	FUploadRawFileSucceed OnUploadRawFileSucceed;

	DECLARE_MULTICAST_DELEGATE_OneParam(FUploadRawFileProgress, float);
	FUploadRawFileProgress OnUploadRawFileProgress;

private:
	UPROPERTY()
	FBKActionUploadRawFileInput Input;
	UPROPERTY()
	FBKActionUploadRawFileOutput Output;

	static FVersionFileEntry MakeVersionFileEntryForUpload(const FBKActionUploadRawFileInput& Input);

	virtual void Execute() override;

	bool CreateEntriesAndOpenFileStreams(TSharedPtr<class BZipArchive> Archive);
	void CloseOpenedFileStreams();

	TArray<TSharedPtr<std::ifstream>> OpenedFileStreams;
};
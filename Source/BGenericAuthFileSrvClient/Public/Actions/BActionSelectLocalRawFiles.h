/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "CoreMinimal.h"
#include "Actions/BActionBase.h"
#include "Data/BModelData.h"
#include "BActionSelectLocalRawFiles.generated.h"

UENUM(BlueprintType)
enum class EBDialogType : uint8
{
	FilesForUpload = 0,
	DirectoryForUpload = 1,
	DirectoryForDownload = 2
};

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FBKActionSelectLocalRawFilesInput : public FBKInputBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	EBDialogType DialogType;
};

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FBKActionSelectLocalRawFilesOutput : public FBKOutputBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FBKActionSelectLocalRawFilesInput RelativeInput;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FString ErrorMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	TArray<FString> SelectedFilePaths;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FString TopAssemblyFileNameIfAny;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	TArray<FString> PotentialCADSoftwareNames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FString FileEntryName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FString SelectedDirectoryPath;

	virtual void SetProperties(FBKOutputBase* Other) override
	{
		FBKActionSelectLocalRawFilesOutput* OtherCasted = (FBKActionSelectLocalRawFilesOutput*)Other;
		*this = *OtherCasted;
	}
};

UCLASS()
class BGENERICAUTHFILESRVCLIENT_API UBKActionSelectLocalRawFiles : public UBKActionBase
{
	GENERATED_BODY()
	
public:
	/*
	* For blueprint usage
	*/
	UFUNCTION(BlueprintCallable, Category = "BGenericAuthFileSrvClient|Actions", DisplayName = "Perform Select Local Raw Files Action", meta = (ExpandEnumAsExecs = "Exec", Latent, LatentInfo = "LatentInfo"))
	static void Perform_Select_Local_Raw_Files_Action_BP(
		const FBKActionSelectLocalRawFilesInput& Input,
		BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
		FBKActionSelectLocalRawFilesOutput& Output,
		struct FLatentActionInfo LatentInfo);

	/*
	* For all in one cpp usage
	*/
	static void Perform_Select_Local_Raw_Files_Action_Cpp(
		const FBKActionSelectLocalRawFilesInput& Input,
		const TFunction<void(const FString&)>& FailedCallback,
		const TFunction<void(const TArray<FString>&, const FString&, const TArray<FString>&, const FString&, const FString&)>& SucceedCallback);

	/*
	* For custom cpp use cases that require multiple listeners binded: NewObject<>(); then bind delegates; then ActionManager->QueueAction.
	*/
	DECLARE_MULTICAST_DELEGATE_OneParam(FSelectLocalRawFilesFailed, const FString&);
	FSelectLocalRawFilesFailed OnSelectLocalRawFilesFailed;

	DECLARE_MULTICAST_DELEGATE_FiveParams(FSelectLocalRawFilesSucceed, const TArray<FString>&, const FString&, const TArray<FString>&, const FString&, const FString&);
	FSelectLocalRawFilesSucceed OnSelectLocalRawFilesSucceed;

private:
	UPROPERTY()
	FBKActionSelectLocalRawFilesInput Input;
	UPROPERTY()
	FBKActionSelectLocalRawFilesOutput Output;

	virtual void Execute() override;

	bool Execute_PickFiles(TArray<FString>& OutFilePaths, FString& OutTopAssemblyFormatFileDialogFriendIfAny, TArray<FString>& OutPotentialCADSoftwares, FString& BestDisplayName, FString& OutSelectedDirectoryPath);
	bool Execute_PickTopAssembly(const FString& TopAssemblyFormatFileDialogFriendIfAny, FString& TopAssemblyFileNameIfAny);
};
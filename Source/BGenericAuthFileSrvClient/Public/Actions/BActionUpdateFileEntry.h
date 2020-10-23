/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "CoreMinimal.h"
#include "Actions/BActionBase.h"
#include "Data/BModelData.h"
#include "BActionUpdateFileEntry.generated.h"

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FBKActionUpdateFileEntryInput : public FBKInputBase
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
	FVersionFileEntry FileEntry;
};

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FBKActionUpdateFileEntryOutput : public FBKOutputBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FBKActionUpdateFileEntryInput RelativeInput;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FString Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	TEnumAsByte<EBKActionErrorCode> ErrorCode;

	virtual void SetProperties(FBKOutputBase* Other) override
	{
		FBKActionUpdateFileEntryOutput* OtherCasted = (FBKActionUpdateFileEntryOutput*)Other;
		*this = *OtherCasted;
	}
};

UCLASS()
class BGENERICAUTHFILESRVCLIENT_API UBKActionUpdateFileEntry : public UBKActionBase
{
	GENERATED_BODY()

public:
	/*
	* For blueprint usage
	* Expected only:
	* fileEntryComments
	*/
	UFUNCTION(BlueprintCallable, Category = "BGenericAuthFileSrvClient|Actions", DisplayName = "Perform UpdateFileEntry Action", meta = (ExpandEnumAsExecs = "Exec", Latent, LatentInfo = "LatentInfo"))
	static void Perform_UpdateFileEntry_Action_BP(
		const FBKActionUpdateFileEntryInput& Input,
		BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
		FBKActionUpdateFileEntryOutput& Output,
		struct FLatentActionInfo LatentInfo);

	/*
	* For all in one cpp usage
	* Expected only:
	* fileEntryComments
	*/
	static void Perform_UpdateFileEntry_Action_Cpp(
		const FBKActionUpdateFileEntryInput& Input,
		const TFunction<void(const FString&, int32)>& FailureCallback,
		const TFunction<void(const FString&)>& SuccessCallback);

	/*
	* For custom cpp use cases that require multiple listeners binded.
	* NewObject<>(); then bind delegates;
	* then InitializeActionDefinition->ThisIsAnUpdateAction->FinalizeActionDefinition
	* then ActionManager->QueueAction.
	*/
	DECLARE_MULTICAST_DELEGATE_TwoParams(FUpdateFileEntryFailed, const FString&, int32);
	FUpdateFileEntryFailed OnUpdateFileEntryFailed;

	DECLARE_MULTICAST_DELEGATE_OneParam(FUpdateFileEntrySucceed, const FString&);
	FUpdateFileEntrySucceed OnUpdateFileEntrySucceed;

private:
	UPROPERTY()
	FBKActionUpdateFileEntryInput Input;
	UPROPERTY()
	FBKActionUpdateFileEntryOutput Output;

	virtual void Execute() override;
};
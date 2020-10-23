/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "CoreMinimal.h"
#include "Actions/BActionBase.h"
#include "Data/BModelData.h"
#include "BActionGetRevisionVersionInfo.generated.h"

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FBKActionGetRevisionVersionInfoInput : public FBKInputBase
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
struct BGENERICAUTHFILESRVCLIENT_API FBKActionGetRevisionVersionInfoOutput : public FBKOutputBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FBKActionGetRevisionVersionInfoInput RelativeInput;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FModelRevisionVersion Version;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FString ErrorMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	TEnumAsByte<EBKActionErrorCode> ErrorCode;

	virtual void SetProperties(FBKOutputBase* Other) override
	{
		FBKActionGetRevisionVersionInfoOutput* OtherCasted = (FBKActionGetRevisionVersionInfoOutput*)Other;
		*this = *OtherCasted;
	}
};

UCLASS()
class UBKActionGetRevisionVersionInfo : public UBKActionBase
{
	GENERATED_BODY()

public:
	/*
	* For blueprint usage
	*/
	UFUNCTION(BlueprintCallable, Category = "BGenericAuthFileSrvClient|Actions", DisplayName = "Perform GetRevisionVersionInfo Action", meta = (ExpandEnumAsExecs = "Exec", Latent, LatentInfo = "LatentInfo"))
	static void Perform_GetRevisionVersionInfo_Action_BP(
		const FBKActionGetRevisionVersionInfoInput& Input,
		BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
		FBKActionGetRevisionVersionInfoOutput& Output,
		struct FLatentActionInfo LatentInfo);

	/*
	* For all in one cpp usage
	*/
	static void Perform_GetRevisionVersionInfo_Action_Cpp(
		const FBKActionGetRevisionVersionInfoInput& Input,
		const TFunction<void(const FString&, int32)>& FailureCallback,
		const TFunction<void(const FModelRevisionVersion&)>& SuccessCallback);

	/*
	* For custom cpp use cases that require multiple listeners binded: NewObject<>(); then bind delegates; then ActionManager->QueueAction.
	*/
	DECLARE_MULTICAST_DELEGATE_TwoParams(FGetRevisionVersionInfoFailed, const FString&, int32);
	FGetRevisionVersionInfoFailed OnGetRevisionVersionInfoFailed;

	DECLARE_MULTICAST_DELEGATE_OneParam(FGetRevisionVersionInfoSucceed, const FModelRevisionVersion&);
	FGetRevisionVersionInfoSucceed OnGetRevisionVersionInfoSucceed;

private:
	UPROPERTY()
	FBKActionGetRevisionVersionInfoInput Input;
	UPROPERTY()
	FBKActionGetRevisionVersionInfoOutput Output;

	virtual void Execute() override;
};
/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "CoreMinimal.h"
#include "Actions/BActionBase.h"
#include "Data/BModelData.h"
#include "BActionDeleteModel.generated.h"

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FBKActionDeleteModelInput : public FBKInputBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FString ModelId;
};

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FBKActionDeleteModelOutput : public FBKOutputBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FBKActionDeleteModelInput RelativeInput;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FString Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	TEnumAsByte<EBKActionErrorCode> ErrorCode;

	virtual void SetProperties(FBKOutputBase* Other) override
	{
		FBKActionDeleteModelOutput* OtherCasted = (FBKActionDeleteModelOutput*)Other;
		*this = *OtherCasted;
	}
};

UCLASS()
class BGENERICAUTHFILESRVCLIENT_API UBKActionDeleteModel : public UBKActionBase
{
	GENERATED_BODY()

public:
	/*
	* For blueprint usage
	*/
	UFUNCTION(BlueprintCallable, Category = "BGenericAuthFileSrvClient|Actions", DisplayName = "Perform DeleteModel Action", meta = (ExpandEnumAsExecs = "Exec", Latent, LatentInfo = "LatentInfo"))
	static void Perform_DeleteModel_Action_BP(
		const FBKActionDeleteModelInput& Input,
		BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
		FBKActionDeleteModelOutput& Output,
		struct FLatentActionInfo LatentInfo);

	/*
	* For all in one cpp usage
	*/
	static void Perform_DeleteModel_Action_Cpp(
		const FBKActionDeleteModelInput& Input,
		const TFunction<void(const FString&, int32)>& FailureCallback,
		const TFunction<void(const FString&)>& SuccessCallback);

	/*
	* For custom cpp use cases that require multiple listeners binded.
	* NewObject<>(); then bind delegates;
	* then ActionManager->QueueAction.
	*/
	DECLARE_MULTICAST_DELEGATE_TwoParams(FDeleteModelFailed, const FString&, int32);
	FDeleteModelFailed OnDeleteModelFailed;

	DECLARE_MULTICAST_DELEGATE_OneParam(FDeleteModelSucceed, const FString&);
	FDeleteModelSucceed OnDeleteModelSucceed;

private:
	UPROPERTY()
	FBKActionDeleteModelInput Input;
	UPROPERTY()
	FBKActionDeleteModelOutput Output;

	virtual void Execute() override;
};
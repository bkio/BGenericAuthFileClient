/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "CoreMinimal.h"
#include "Actions/BActionBase.h"
#include "Data/BModelData.h"
#include "BActionUpdateModelInfo.generated.h"

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FBKActionUpdateModelInfoInput : public FBKInputBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FString ModelId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FModelInfo ModelInfo;
};

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FBKActionUpdateModelInfoOutput : public FBKOutputBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FBKActionUpdateModelInfoInput RelativeInput;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FString Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	TEnumAsByte<EBKActionErrorCode> ErrorCode;

	virtual void SetProperties(FBKOutputBase* Other) override
	{
		FBKActionUpdateModelInfoOutput* OtherCasted = (FBKActionUpdateModelInfoOutput*)Other;
		*this = *OtherCasted;
	}
};

UCLASS()
class BGENERICAUTHFILESRVCLIENT_API UBKActionUpdateModelInfo : public UBKActionBase
{
	GENERATED_BODY()

public:
	/*
	* For blueprint usage
	*/
	UFUNCTION(BlueprintCallable, Category = "BGenericAuthFileSrvClient|Actions", DisplayName = "Perform UpdateModelInfo Action", meta = (ExpandEnumAsExecs = "Exec", Latent, LatentInfo = "LatentInfo"))
	static void Perform_UpdateModelInfo_Action_BP(
		const FBKActionUpdateModelInfoInput& Input,
		BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
		FBKActionUpdateModelInfoOutput& Output,
		struct FLatentActionInfo LatentInfo);

	/*
	* For all in one cpp usage
	*/
	static void Perform_UpdateModelInfo_Action_Cpp(
		const FBKActionUpdateModelInfoInput& Input,
		const TFunction<void(const FString&, int32)>& FailureCallback,
		const TFunction<void(const FString&)>& SuccessCallback);

	/*
	* For custom cpp use cases that require multiple listeners binded.
	* NewObject<>(); then bind delegates;
	* then InitializeActionDefinition->ThisIsAnUpdateAction->FinalizeActionDefinition
	* then ActionManager->QueueAction.
	*/
	DECLARE_MULTICAST_DELEGATE_TwoParams(FUpdateModelInfoFailed, const FString&, int32);
	FUpdateModelInfoFailed OnUpdateModelInfoFailed;

	DECLARE_MULTICAST_DELEGATE_OneParam(FUpdateModelInfoSucceed, const FString&);
	FUpdateModelInfoSucceed OnUpdateModelInfoSucceed;

private:
	UPROPERTY()
	FBKActionUpdateModelInfoInput Input;
	UPROPERTY()
	FBKActionUpdateModelInfoOutput Output;

	virtual void Execute() override;
};
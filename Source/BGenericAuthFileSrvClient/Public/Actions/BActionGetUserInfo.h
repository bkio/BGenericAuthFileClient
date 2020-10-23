/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "CoreMinimal.h"
#include "Actions/BActionBase.h"
#include "Data/BUserData.h"
#include "BActionGetUserInfo.generated.h"

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FBKActionGetUserInfoOutput : public FBKOutputBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FBKInputBase RelativeInput;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FUser UserInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FString ErrorMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	TEnumAsByte<EBKActionErrorCode> ErrorCode;

	virtual void SetProperties(FBKOutputBase* Other) override
	{
		FBKActionGetUserInfoOutput* OtherCasted = (FBKActionGetUserInfoOutput*)Other;
		*this = *OtherCasted;
	}
};

UCLASS()
class BGENERICAUTHFILESRVCLIENT_API UBKActionGetUserInfo : public UBKActionBase
{
	GENERATED_BODY()

public:
	/*
	* For blueprint usage
	*/
	UFUNCTION(BlueprintCallable, Category = "BGenericAuthFileSrvClient|Actions", DisplayName = "Perform GetUserInfo Action", meta = (ExpandEnumAsExecs = "Exec", Latent, LatentInfo = "LatentInfo"))
	static void Perform_GetUserInfo_Action_BP(
		const FBKInputBase& Input,
		BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
		FBKActionGetUserInfoOutput& Output,
		struct FLatentActionInfo LatentInfo);

	/*
	* For all in one cpp usage
	*/
	static void Perform_GetUserInfo_Action_Cpp(
		const TFunction<void(const FString&, int32)>& GetUserInfoFailedCallback,
		const TFunction<void(const FUser&)>& GetUserInfoSucceedCallback);

	/*
	* For custom cpp use cases that require multiple listeners binded: NewObject<>(); then bind delegates; then ActionManager->QueueAction.
	*/
	DECLARE_MULTICAST_DELEGATE_TwoParams(FGetUserInfoFailed, const FString&, int32);
	FGetUserInfoFailed OnGetUserInfoFailed;

	DECLARE_MULTICAST_DELEGATE_OneParam(FRVGetUserInfoSucceed, const FUser&);
	FRVGetUserInfoSucceed OnGetUserInfoSucceed;

private:
	UPROPERTY()
	FBKInputBase Input;
	UPROPERTY()
	FBKActionGetUserInfoOutput Output;

	virtual void Execute() override;
};
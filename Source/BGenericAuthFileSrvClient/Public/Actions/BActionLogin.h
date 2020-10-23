/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "CoreMinimal.h"
#include "BActionBase.h"
#include "BActionLogin.generated.h"

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FBKActionLoginOutput : public FBKOutputBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FBKInputBase RelativeInput;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FString Message;

	virtual void SetProperties(FBKOutputBase* Other) override
	{
		FBKActionLoginOutput* OtherCasted = (FBKActionLoginOutput*)Other;
		*this = *OtherCasted;
	}
};

UCLASS()
class BGENERICAUTHFILESRVCLIENT_API UBKActionLogin : public UBKActionBase
{
	GENERATED_BODY()
	
public:
	/*
	* For blueprint usage
	*/
	UFUNCTION(BlueprintCallable, Category = "BGenericAuthFileSrvClient|Actions", DisplayName = "Perform Login Action", meta = (ExpandEnumAsExecs = "Exec", Latent, LatentInfo = "LatentInfo"))
	static void Perform_Login_Action_BP(
		const FBKInputBase& Input,
		BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
		FBKActionLoginOutput& Output,
		struct FLatentActionInfo LatentInfo);

	/*
	* For all in one cpp usage
	*/
	static void Perform_Login_Action_Cpp(
		const TFunction<void(const FString&)>& LoginFailedCallback, 
		const TFunction<void(const FString&)>& LoginSucceedCallback);

	/*
	* For custom cpp use cases that require multiple listeners binded: NewObject<>(); then bind delegates; then ActionManager->QueueAction.
	*/
	DECLARE_MULTICAST_DELEGATE_OneParam(FLoginFailed, const FString&);
	FLoginFailed OnLoginFailed;

	DECLARE_MULTICAST_DELEGATE_OneParam(FRVLoginSucceed, const FString&);
	FRVLoginSucceed OnLoginSucceed;

private:
	UPROPERTY()
	FBKInputBase Input;
	UPROPERTY()
	FBKActionLoginOutput Output;

	virtual void Execute() override;
};
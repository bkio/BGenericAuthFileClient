/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "CoreMinimal.h"
#include "Actions/BActionBase.h"
#include "BActionLogout.generated.h"

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FBKActionLogoutOutput : public FBKOutputBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FBKInputBase RelativeInput;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FString Message;

	virtual void SetProperties(FBKOutputBase* Other) override
	{
		FBKActionLogoutOutput* OtherCasted = (FBKActionLogoutOutput*)Other;
		*this = *OtherCasted;
	}
};

UCLASS()
class BGENERICAUTHFILESRVCLIENT_API UBKActionLogout : public UBKActionBase
{
	GENERATED_BODY()

public:
	/*
	* For blueprint usage
	*/
	UFUNCTION(BlueprintCallable, Category = "BGenericAuthFileSrvClient|Actions", DisplayName = "Perform Logout Action", meta = (ExpandEnumAsExecs = "Exec", Latent, LatentInfo = "LatentInfo"))
	static void Perform_Logout_Action_BP(
		const FBKInputBase& Input,
		BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
		FBKActionLogoutOutput& Output,
		struct FLatentActionInfo LatentInfo);
	//In case we add some http calls in the future to this function; better to make it latent in advance.

	/*
	* For all in one cpp usage
	*/
	static void Perform_Logout_Action_Cpp(
		const TFunction<void(const FString&)>& LogoutFailedCallback,
		const TFunction<void(const FString&)>& LogoutSucceedCallback);

	/*
	* For custom cpp use cases that require multiple listeners binded: NewObject<>(); then bind delegates; then ActionManager->QueueAction.
	*/
	DECLARE_MULTICAST_DELEGATE_OneParam(FLogoutFailed, const FString&);
	FLogoutFailed OnLogoutFailed;

	DECLARE_MULTICAST_DELEGATE_OneParam(FLogoutSucceed, const FString&);
	FLogoutSucceed OnLogoutSucceed;

private:
	UPROPERTY()
	FBKInputBase Input;
	UPROPERTY()
	FBKActionLogoutOutput Output;

	virtual void Execute() override;
};
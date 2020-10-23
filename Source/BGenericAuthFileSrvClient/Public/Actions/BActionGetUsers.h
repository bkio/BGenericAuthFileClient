/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "CoreMinimal.h"
#include "BActionBase.h"
#include "Data/BModelData.h"
#include "BActionGetUsers.generated.h"


USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FBKActionGetUsersOutput : public FBKOutputBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
		FString Message;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
		int ErrorCode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
		FBKInputBase RelativeInput;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
		FRegisteredEmailAddresses EmailAddresses;

	virtual void SetProperties(FBKOutputBase* Other) override
	{
		FBKActionGetUsersOutput* OtherCasted = (FBKActionGetUsersOutput*)Other;
		*this = *OtherCasted;
	}

};

UCLASS()
class BGENERICAUTHFILESRVCLIENT_API UBKActionGetUsers: public UBKActionBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "BGenericAuthFileSrvClient|Actions", DisplayName = "Perform Get User EmailAddresses Action", meta = (ExpandEnumAsExecs = "Exec", Latent, LatentInfo = "LatentInfo"))
		static void Perform_GetUserEmailAddresses_Action_BP(
			const FBKInputBase& Input,
			BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
			FBKActionGetUsersOutput& Output,
			struct FLatentActionInfo LatentInfo);

	DECLARE_MULTICAST_DELEGATE_TwoParams(FGetUsersFailed, const FString&, int32);
	FGetUsersFailed OnGetUsersFailed;

	DECLARE_MULTICAST_DELEGATE_OneParam(FGetUsersSucceed, const FRegisteredEmailAddresses&);
	FGetUsersSucceed OnGetUsersSucceed;

private:
	virtual void Execute() override;


	UPROPERTY()
		FBKInputBase Input;
	UPROPERTY()
		FBKActionGetUsersOutput Output;
};
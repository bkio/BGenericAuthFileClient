/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "CoreMinimal.h"
#include "Actions/BActionBase.h"
#include "Data/BModelData.h"
#include "BActionRemoveSharedWithUser.generated.h"


USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FBKActionRemoveSharedModelInput : public FBKInputBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
		FString ModelId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
		FString UserId;
};

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FBKActionDeleteSharedModelOutput : public FBKOutputBase
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
		FString ModelId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
		FString UserId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
		FString Message;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
		TEnumAsByte<EBKActionErrorCode> ErrorCode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
		FBKActionRemoveSharedModelInput RelativeInput;

	virtual void SetProperties(FBKOutputBase* Other) override
	{
		FBKActionDeleteSharedModelOutput* OtherCasted = (FBKActionDeleteSharedModelOutput*)Other;
		*this = *OtherCasted;
	}
};

UCLASS()
class BGENERICAUTHFILESRVCLIENT_API UBKActionDeleteSharedModel : public UBKActionBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "BGenericAuthFileSrvClient|Actions", DisplayName = "Perform Remove Shared Model Action", meta = (ExpandEnumAsExecs = "Exec", Latent, LatentInfo = "LatentInfo"))
		static void Perform_RemoveSharedModel_Action_BP(
			const FBKActionRemoveSharedModelInput& Input,
			BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
			FBKActionDeleteSharedModelOutput& Output,
			struct FLatentActionInfo LatentInfo);

	DECLARE_MULTICAST_DELEGATE_TwoParams(FDeleteSharedModelFailed, const FString&, int32);
	FDeleteSharedModelFailed OnDeleteSharedModelFailed;

	DECLARE_MULTICAST_DELEGATE(FDeleteSharedModelSucceed);
	FDeleteSharedModelSucceed OnDeleteSharedModelSucceed;

private:
	virtual void Execute() override;

	UPROPERTY()
		FBKActionRemoveSharedModelInput Input;
	UPROPERTY()
		FBKActionDeleteSharedModelOutput Output;
};
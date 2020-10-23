/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "CoreMinimal.h"
#include "Actions/BActionBase.h"
#include "Data/BModelData.h"
#include "BActionShareModel.generated.h"


USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FBKActionShareModelInput : public FBKInputBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
		FString ModelId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
		FShareInfo ShareInfo;
};

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FBKActionShareModelOutput : public FBKOutputBase
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
		FString ModelId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
		FString Message;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
		TEnumAsByte<EBKActionErrorCode> ErrorCode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
		FBKActionShareModelInput RelativeInput;

	virtual void SetProperties(FBKOutputBase* Other) override
	{
		FBKActionShareModelOutput* OtherCasted = (FBKActionShareModelOutput*)Other;
		*this = *OtherCasted;
	}
};

UCLASS()
class BGENERICAUTHFILESRVCLIENT_API UBKActionShareModel : public UBKActionBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "BGenericAuthFileSrvClient|Actions", DisplayName = "Perform Share Model Action", meta = (ExpandEnumAsExecs = "Exec", Latent, LatentInfo = "LatentInfo"))
		static void Perform_ShareModel_Action_BP(
			const FBKActionShareModelInput& Input,
			BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
			FBKActionShareModelOutput& Output,
			struct FLatentActionInfo LatentInfo);

	DECLARE_MULTICAST_DELEGATE_TwoParams(FShareModelFailed, const FString&, int32);
	FShareModelFailed OnShareModelFailed;

	DECLARE_MULTICAST_DELEGATE_OneParam(FShareModelSucceed, const FString&);
	FShareModelSucceed OnShareModelSucceed;

private:
	virtual void Execute() override;

	UPROPERTY()
		FBKActionShareModelInput Input;
	UPROPERTY()
		FBKActionShareModelOutput Output;
};
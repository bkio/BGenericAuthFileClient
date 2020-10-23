/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "CoreMinimal.h"
#include "Actions/BActionBase.h"
#include "BActionRemoveTabbedWidget.generated.h"

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FBKActionRemoveTabbedWidgetInput : public FBKInputBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FString OutUniqueId;
};

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FBKActionRemoveTabbedWidgetOutput : public FBKOutputBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FBKActionRemoveTabbedWidgetInput RelativeInput;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FString ErrorMessage;

	virtual void SetProperties(FBKOutputBase* Other) override
	{
		FBKActionRemoveTabbedWidgetOutput* OtherCasted = (FBKActionRemoveTabbedWidgetOutput*)Other;
		*this = *OtherCasted;
	}
};

UCLASS()
class BGENERICAUTHFILESRVCLIENT_API UBKActionRemoveTabbedWidget : public UBKActionBase
{
	GENERATED_BODY()

public:
	/*
	* For blueprint usage
	*/
	UFUNCTION(BlueprintCallable, Category = "BGenericAuthFileSrvClient|Actions", DisplayName = "Perform Remove Tabbed Widget Action", meta = (ExpandEnumAsExecs = "Exec", Latent, LatentInfo = "LatentInfo"))
	static void Perform_RemoveTabbedWidget_Action_BP(
		const FBKActionRemoveTabbedWidgetInput& Input,
		BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
		FBKActionRemoveTabbedWidgetOutput& Output,
		struct FLatentActionInfo LatentInfo);

	/*
	* For all in one cpp usage
	*/
	static void Perform_RemoveTabbedWidget_Action_Cpp(
		const FBKActionRemoveTabbedWidgetInput& Input,
		const TFunction<void(const FString&)>& FailedCallback,
		const TFunction<void()>& SucceedCallback);

	/*
	* For custom cpp use cases that require multiple listeners binded: NewObject<>(); then bind delegates; then ActionManager->QueueAction.
	*/
	DECLARE_MULTICAST_DELEGATE_OneParam(FRemoveTabbedWidgetFailed, const FString&);
	FRemoveTabbedWidgetFailed OnRemoveTabbedWidgetFailed;

	DECLARE_MULTICAST_DELEGATE(FRemoveTabbedWidgetSucceed);
	FRemoveTabbedWidgetSucceed OnRemoveTabbedWidgetSucceed;

private:
	UPROPERTY()
	FBKActionRemoveTabbedWidgetInput Input;
	UPROPERTY()
	FBKActionRemoveTabbedWidgetOutput Output;

	virtual void Execute() override;
};
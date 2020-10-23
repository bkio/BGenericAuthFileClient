/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "CoreMinimal.h"
#include "Actions/BActionBase.h"
#include "BActionCreateTabbedWidget.generated.h"

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FBKActionCreateTabbedWidgetInput : public FBKInputBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	class UEditorUtilityWidgetBlueprint* WidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FString TabTitle;
};

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FBKActionCreateTabbedWidgetOutput : public FBKOutputBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FBKActionCreateTabbedWidgetInput RelativeInput;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FString OutUniqueId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions")
	FString ErrorMessage;

	virtual void SetProperties(FBKOutputBase* Other) override
	{
		FBKActionCreateTabbedWidgetOutput* OtherCasted = (FBKActionCreateTabbedWidgetOutput*)Other;
		*this = *OtherCasted;
	}
};

UCLASS()
class BGENERICAUTHFILESRVCLIENT_API UBKActionCreateTabbedWidget : public UBKActionBase
{
	GENERATED_BODY()

public:
	/*
	* For blueprint usage
	*/
	UFUNCTION(BlueprintCallable, Category = "BGenericAuthFileSrvClient|Actions", DisplayName = "Perform Create Tabbed Widget Action", meta = (ExpandEnumAsExecs = "Exec", Latent, LatentInfo = "LatentInfo"))
		static void Perform_CreateTabbedWidget_Action_BP(
			const FBKActionCreateTabbedWidgetInput& Input,
			BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
			FBKActionCreateTabbedWidgetOutput& Output,
			struct FLatentActionInfo LatentInfo);

	/*
	* For all in one cpp usage
	*/
	static void Perform_CreateTabbedWidget_Action_Cpp(
		const FBKActionCreateTabbedWidgetInput& Input,
		const TFunction<void(const FString&)>& FailedCallback,
		const TFunction<void()>& SucceedCallback);

	/*
	* For custom cpp use cases that require multiple listeners binded: NewObject<>(); then bind delegates; then ActionManager->QueueAction.
	*/
	DECLARE_MULTICAST_DELEGATE_OneParam(FCreateTabbedWidgetFailed, const FString&);
	FCreateTabbedWidgetFailed OnCreateTabbedWidgetFailed;

	DECLARE_MULTICAST_DELEGATE(FCreateTabbedWidgetSucceed);
	FCreateTabbedWidgetSucceed OnCreateTabbedWidgetSucceed;

private:
	UPROPERTY()
		FBKActionCreateTabbedWidgetInput Input;
	UPROPERTY()
		FBKActionCreateTabbedWidgetOutput Output;

	virtual void Execute() override;
};
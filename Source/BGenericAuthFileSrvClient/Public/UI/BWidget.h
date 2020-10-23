/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "BWidget.generated.h"

enum class EViewType : uint8;

UENUM(BlueprintType)
enum class EWidgetAuthenticatable : uint8
{
	NotSet = 0,
	NotAuthenticatable = 1,
	Authenticatable = 2
};


UCLASS()
class BGENERICAUTHFILESRVCLIENT_API UBKWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()
};

UCLASS()
class BGENERICAUTHFILESRVCLIENT_API UBKViewWidget : public UBKWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BLueprintCallable, Category = "BGenericAuthFileSrvClient|Widgets")
	void OnAuthenticationStatusChanged(bool bNewAuthenticationStatus);

	void OnOwnerBlueprintCompiled();
	void OnTabBeingClosed();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BGenericAuthFileSrvClient|Widgets")
	bool bFocusIsCurrentView = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BGenericAuthFileSrvClient|Widgets")
	EViewType WidgetViewType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BGenericAuthFileSrvClient|Widgets")
	EWidgetAuthenticatable WidgetAuthenticatable = EWidgetAuthenticatable::NotSet;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "BGenericAuthFileSrvClient|Widgets")
	void AuthenticationStatusChangedAndFocusViewIsThis(bool bNewAuthenticationStatus);

	UFUNCTION(BlueprintImplementableEvent, Category = "BGenericAuthFileSrvClient|Widgets")
	void FocusViewIsThisWidget();

	UFUNCTION(BlueprintImplementableEvent, Category = "BGenericAuthFileSrvClient|Widgets")
	void FocusViewIsAnotherWidget();

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	uint8 LastProcessedAuthenticationStatus = 2;//0: false 1: true 2: none (default)

	UFUNCTION()
	void OnViewChanged_Internal(int32 ViewIndex);

	void InvalidateWidgetBounds();
};
/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "Managers/BManager.h"
#include "BViewManager.generated.h"

enum class EMapChangeType : uint8;

UENUM(BlueprintType)
enum class EViewType : uint8
{
	None = 0,
	LoginView = 1,
	LoginWaitView = 2,
	LoggedInView = 3
};

class TabbedUIWidgetProperties
{

public: 
	bool bIsMainWidget;
	FString UniqueId;

	TWeakPtr<class SDockTab> TabWeakPtr;

	TWeakObjectPtr<class UEditorUtilityWidgetBlueprint> TabAssetBP;
	TWeakObjectPtr<class UEditorUtilityWidget> TabAssetBPLastProcessedWidget;

	FName TabRegistrationName;

	FString TabTitle;

	class FDelegateHandle LastOnCompiledDelegate;

	TabbedUIWidgetProperties() {}
	TabbedUIWidgetProperties(bool bInIsMainWidget, const FString& InUniqueId, class UEditorUtilityWidgetBlueprint* AssetBP, const FString& Title, const FName& RegistrationName);

	static void Initialize(bool bMainWidgetTab, TSharedPtr<TabbedUIWidgetProperties> PropsPtr);
	~TabbedUIWidgetProperties();
};

UCLASS()
class BGENERICAUTHFILESRVCLIENT_API UBKViewManager : public UBKManager
{
	GENERATED_BODY()
	
protected:
	virtual void OnConstruct() override;
	virtual void OnDestruct() override;

public:
	UBKViewManager(); //Use only for being able to call ConstructorHelpers::FObjectFinder

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FViewChanged, int32, ViewIndex);
	UPROPERTY(BlueprintAssignable)
	FViewChanged OnViewChanged;

	UFUNCTION(BlueprintCallable, Category = "BGenericAuthFileSrvClient|View Manager")
	void SelectView(EViewType View);

	UFUNCTION(BlueprintCallable, Category = "BGenericAuthFileSrvClient|View Manager")
	EViewType GetCurrentView();

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSelectMainViewAndValidateUserCalled);
	UPROPERTY(BlueprintAssignable)
	FSelectMainViewAndValidateUserCalled OnSelectMainViewAndValidateUserCalled;

	UFUNCTION(BlueprintCallable, Category = "BGenericAuthFileSrvClient|View Manager")
	UEditorUtilityWidget* GetMainWidget() const;

	void CreateTabbedWidget(FString& OutUniqueId, class UEditorUtilityWidgetBlueprint* WidgetBP, const FString& TabTitle, TFunction<void(bool, const FString&)> OnComplete = nullptr);
	void RemoveTabbedWidget(const FString& UniqueId, TFunction<void(bool, const FString&)> OnComplete = nullptr);

	UFUNCTION(BlueprintCallable, Category = "BGenericAuthFileSrvClient|View Manager")
	UEditorUtilityWidget* GetCurrentTabbedWidget(const FString& UniqueId) const;

	friend class TabbedUIWidgetProperties;

private:
	EViewType CurrentView;

	static TSharedPtr<class FSlateStyleSet> StyleSet;
	void RegisterStyleSet();
	void UnregisterStyleSet();

	TSharedPtr<TabbedUIWidgetProperties> BaseUIWidgetProps;
	TArray<TSharedPtr<TabbedUIWidgetProperties>> AdditionalUIWidgetsProps;

	void RegisterTab(bool bIsMainWidget, TSharedPtr<TabbedUIWidgetProperties> TabbedWidgetProps, TFunction<void(bool, const FString&)> OnComplete = nullptr);
	void RegisterTab_Internal(bool bIsMainWidget, TSharedPtr<TabbedUIWidgetProperties> TabbedWidgetProps);
	void UnregisterMainTab();
	void UnregisterAdditionalTab(TSharedPtr<TabbedUIWidgetProperties> TabbedWidgetProps);
	void Unregister_Internal(TSharedPtr<TabbedUIWidgetProperties> TabbedWidgetProps);
	
	void OnTabBeingClosed(TSharedRef<class SDockTab> TabBeingClosed);

	virtual void OnFirstTimeMapLoaded() override;
	virtual void OnNonFirstTimeMapLoaded() override;
	virtual void OnMapIsBeingTornDown() override;

	virtual void Tick(float DeltaTime) override;
	void EnsureTabSizeInTick(TWeakPtr<class SDockTab> TabWeakPtr, const FVector2D& WindowSize);

	void SelectMainViewAndTryValidatingUser();

	float ValidateUserRetryTimer = 0.0f;
};
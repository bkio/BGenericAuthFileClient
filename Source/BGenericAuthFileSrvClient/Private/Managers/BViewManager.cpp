/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "Managers/BViewManager.h"
#include "Managers/BNetworkManager.h"
#include "Managers/BManagerStore.h"
#include "UObject/ConstructorHelpers.h"
#include "Modules/ModuleManager.h"
#include "EditorUtilityWidget.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "LevelEditor.h"
#include "EditorUtilitySubsystem.h"
#include "IBlutilityModule.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "UI/BWidget.h"
#include "Network/BCancellationToken.h"

#define LOCTEXT_NAMESPACE "EditorUtilitySubsystem"

// Validate user every minute
#define VALIDATE_RETRY_TIME 60.0f

UBKViewManager::UBKViewManager() //Use only for being able to call ConstructorHelpers::FObjectFinder
{
	if (HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject)) return;

	static ConstructorHelpers::FObjectFinder<UWidgetBlueprint> BaseUIWidgetAssetFinder(TEXT("/BGenericAuthFileSrvClient/Widgets/UI_TabbedViews/UI_BK_MainView.UI_BK_MainView"));

	if (BaseUIWidgetAssetFinder.Succeeded())
	{
		BaseUIWidgetProps = MakeShareable<TabbedUIWidgetProperties>(new TabbedUIWidgetProperties(
			true,
			TEXT("BaseUIWidgetProps"),
			(UEditorUtilityWidgetBlueprint*)BaseUIWidgetAssetFinder.Object,
			TEXT("BTwinDesigner"),
			FName(*(BaseUIWidgetAssetFinder.Object->GetPathName() + LOCTEXT("ActiveTabSuffix", "_ActiveTab").ToString()))));
		TabbedUIWidgetProperties::Initialize(true, BaseUIWidgetProps);
	}
}

TabbedUIWidgetProperties::TabbedUIWidgetProperties(
	bool bInIsMainWidget, 
	const FString& InUniqueId, 
	UEditorUtilityWidgetBlueprint* AssetBP, 
	const FString& Title, 
	const FName& RegistrationName)
{
	bIsMainWidget = bInIsMainWidget;
	UniqueId = InUniqueId;
	TabTitle = Title;
	TabRegistrationName = RegistrationName;
	TabAssetBP = TWeakObjectPtr<UEditorUtilityWidgetBlueprint>(AssetBP);
}

void TabbedUIWidgetProperties::Initialize(bool bMainWidgetTab, TSharedPtr<TabbedUIWidgetProperties> PropsPtr)
{
	if (!PropsPtr.IsValid() || !PropsPtr->TabAssetBP.IsValid()) return;

	TWeakPtr<TabbedUIWidgetProperties> WeakPropsPtr = PropsPtr;
	FString UniqueId = PropsPtr->UniqueId;

	PropsPtr->LastOnCompiledDelegate = PropsPtr->TabAssetBP->OnCompiled().AddLambda([bMainWidgetTab, UniqueId, WeakPropsPtr]
		(UBlueprint* CompiledBP)
		{
			UE_LOG(LogTemp, Warning, TEXT("bBeingCompiled:%d bHasBeenRegenerated:%d bIsNewlyCreated:%d bQueuedForCompilation:%d Status:%d"), CompiledBP->bBeingCompiled, CompiledBP->bHasBeenRegenerated, CompiledBP->bIsNewlyCreated, CompiledBP->bQueuedForCompilation, CompiledBP->Status);
			if (!WeakPropsPtr.IsValid()) return;
			TSharedPtr<TabbedUIWidgetProperties> Pinned = WeakPropsPtr.Pin();

			if (Pinned->TabAssetBPLastProcessedWidget.IsValid())
			{
				((UBKViewWidget*)Pinned->TabAssetBPLastProcessedWidget.Get())->OnOwnerBlueprintCompiled();
			}

			if (UBKViewManager* ViewManager = UBKManagerStore::GetViewManager())
			{
				if (!bMainWidgetTab)
				{
					ViewManager->RemoveTabbedWidget(UniqueId);
				}

				//We do not know if this or Blueprint's OnCompiled binded delegate is called first. Therefore a delay is needed.
				if (UWorld* EdWorld = GEditor->GetEditorWorldContext().World())
				{
					if (EdWorld->IsValidLowLevel() && !EdWorld->IsPendingKillOrUnreachable())
					{
						FTimerHandle TimerHandle;
						FTimerDelegate TimerDelegate;

						TimerDelegate.BindLambda([WeakPropsPtr]()
							{
								//Widget is recreated here for sure.

								if (!WeakPropsPtr.IsValid()) return;
								TSharedPtr<TabbedUIWidgetProperties> Pinned = WeakPropsPtr.Pin();
								if (!Pinned->TabAssetBP.IsValid()) return;

								Pinned->TabAssetBPLastProcessedWidget = Pinned->TabAssetBP->GetCreatedWidget();

								if (Pinned->bIsMainWidget)
								{
									if (UBKViewManager* ViewManager = UBKManagerStore::GetViewManager())
									{
										ViewManager->SelectMainViewAndTryValidatingUser();
									}
								}
							});
						EdWorld->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 0.01f, false);
					}
				}
			}
		});
}
TabbedUIWidgetProperties::~TabbedUIWidgetProperties()
{
	if (!TabAssetBP.IsValid()) return;
	if (!LastOnCompiledDelegate.IsValid()) return;
	TabAssetBP->OnCompiled().Remove(LastOnCompiledDelegate);
}

void UBKViewManager::OnConstruct()
{
	Super::OnConstruct();
}

void UBKViewManager::OnDestruct()
{
	if (WasConstructSuccessful())
	{
		UnregisterStyleSet();

		UnregisterMainTab();
		for (int32 i = 0; i < AdditionalUIWidgetsProps.Num(); i++)
		{
			UnregisterAdditionalTab(AdditionalUIWidgetsProps[i]);
		}
	}

	Super::OnDestruct();
}

void UBKViewManager::OnFirstTimeMapLoaded()
{
	RegisterStyleSet();
	RegisterTab(true, BaseUIWidgetProps);
}

void UBKViewManager::OnNonFirstTimeMapLoaded()
{
	for (int32 i = AdditionalUIWidgetsProps.Num() - 1; i >= 0; i--)
	{
		UnregisterAdditionalTab(AdditionalUIWidgetsProps[i]);
	}
	SelectMainViewAndTryValidatingUser();
}

void UBKViewManager::OnMapIsBeingTornDown()
{
}

TSharedPtr<FSlateStyleSet> UBKViewManager::StyleSet = nullptr;

void UBKViewManager::RegisterStyleSet()
{
	// Only register once
	/*if (StyleSet.IsValid())
	{
		return;
	}*/

	//StyleSet = MakeShareable(new FSlateStyleSet("BMainTabIconStyle"));

	//StyleSet->Set("BMainTabIcon", new FSlateImageBrush(StyleSet->RootToContentDir(TEXT("logo_40"), TEXT(".png")), FVector2D(40.0f, 40.0f)));
	//StyleSet->Set("BMainTabIcon.Small", new FSlateImageBrush(StyleSet->RootToContentDir(TEXT("logo_20"), TEXT(".png")), FVector2D(20.0f, 20.0f)));

	//FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());
}

void UBKViewManager::UnregisterStyleSet()
{
	/*if (StyleSet.IsValid())
	{
		//FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
		ensure(StyleSet.IsUnique());
		StyleSet.Reset();
	}*/
}

void UBKViewManager::SelectView(EViewType View)
{
	//if (CurrentView == View) return; This one causes closing the main tab in login view to not respawn the widget.
	CurrentView = View;

	OnViewChanged.Broadcast((int32)CurrentView);
}

EViewType UBKViewManager::GetCurrentView()
{
	return CurrentView;
}

void UBKViewManager::SelectMainViewAndTryValidatingUser()
{
	SelectView(EViewType::LoginView);

	if (BaseUIWidgetProps.IsValid() && BaseUIWidgetProps->TabAssetBPLastProcessedWidget.IsValid())
	{
		if (UBKNetworkManager* NetworkManager = UBKManagerStore::GetNetworkManager())
		{
			if (NetworkManager->ValidateUser(true, BKCancellationTokenWeakWrapper::MakeFromWidget(BaseUIWidgetProps->TabAssetBPLastProcessedWidget.Get())))
			{
				OnSelectMainViewAndValidateUserCalled.Broadcast();
			}
		}
	}
}

UEditorUtilityWidget* UBKViewManager::GetMainWidget() const
{
	if (BaseUIWidgetProps.IsValid())
	{
		return BaseUIWidgetProps->TabAssetBPLastProcessedWidget.Get();
	}
	return nullptr;
}

void UBKViewManager::CreateTabbedWidget(FString& OutUniqueId, UEditorUtilityWidgetBlueprint* WidgetBP, const FString& TabTitle, TFunction<void(bool, const FString&)> OnComplete)
{
	if (!WidgetBP || !WidgetBP->IsValidLowLevel() || WidgetBP->IsPendingKillOrUnreachable())
	{
		if (OnComplete)
		{
			OnComplete(false, TEXT("Widget class is invalid."));
		}
		return;
	}

	if (BaseUIWidgetProps.IsValid() && BaseUIWidgetProps->TabAssetBP.IsValid() && WidgetBP->GetPathName() == BaseUIWidgetProps->TabAssetBP->GetPathName())
	{
		if (OnComplete)
		{
			OnComplete(false, TEXT("Widget class must be different than main widget class."));
		}
		return;
	}

	for (auto Current : AdditionalUIWidgetsProps)
	{
		if (Current.IsValid() && Current->TabAssetBP.IsValid())
		{
			if (Current->TabAssetBP->GetPathName() == WidgetBP->GetPathName())
			{
				if (OnComplete)
				{
					OnComplete(false, TEXT("Widget class already exists in another tabbed widget."));
				}
				return;
			}
		}
	}

	OutUniqueId = GenerateRandomANSIString();

	TSharedPtr<TabbedUIWidgetProperties> NewPropsPtr = MakeShareable<TabbedUIWidgetProperties>(new TabbedUIWidgetProperties(
		false,
		OutUniqueId,
		WidgetBP,
		TabTitle,
		FName(*((WidgetBP->GetPathName() + LOCTEXT("ActiveTabSuffix", "_ActiveTab").ToString())))));

	TabbedUIWidgetProperties::Initialize(false, NewPropsPtr);

	RegisterTab(false, NewPropsPtr, OnComplete);
}

void UBKViewManager::RemoveTabbedWidget(const FString& UniqueId, TFunction<void(bool, const FString&)> OnComplete)
{
	for (auto Current : AdditionalUIWidgetsProps)
	{
		if (Current.IsValid())
		{
			if (Current->UniqueId == UniqueId)
			{
				UnregisterAdditionalTab(Current);
				if (OnComplete)
					OnComplete(true, TEXT(""));
				return;
			}
		}
	}
	if (OnComplete)
		OnComplete(false, TEXT("Tabbed widget does not exist."));
}

UEditorUtilityWidget* UBKViewManager::GetCurrentTabbedWidget(const FString& UniqueId) const
{
	for (auto Current : AdditionalUIWidgetsProps)
	{
		if (Current.IsValid())
		{
			if (Current->UniqueId == UniqueId)
			{
				return Current->TabAssetBPLastProcessedWidget.Get();
			}
		}
	}
	return nullptr;
}

void UBKViewManager::RegisterTab(bool bIsMainWidget, TSharedPtr<TabbedUIWidgetProperties> TabbedWidgetProps, TFunction<void(bool, const FString&)> OnComplete)
{
	if (!TabbedWidgetProps.IsValid() || !TabbedWidgetProps->TabAssetBP.IsValid()) return;

	if (UWorld* EdWorld = GEditor->GetEditorWorldContext().World())
	{
		if (EdWorld->IsValidLowLevel() && !EdWorld->IsPendingKillOrUnreachable())
		{
			FTimerHandle TimerHandle;
			FTimerDelegate TimerDelegate;

			TWeakObjectPtr<UBKViewManager> ThisPtr(this);

			//UEditorUtilityWidgetBlueprint might remove the widget from LoadedScriptUI list. The delay is for being sure that delegate is called after UEditorUtilityWidgetBlueprint->SetOnTabClosed handler.
			TimerDelegate.BindLambda([ThisPtr, bIsMainWidget, TabbedWidgetProps, OnComplete]()
				{
					if (!ThisPtr.IsValid())
					{
						if (OnComplete)
						{
							OnComplete(false, TEXT("World has changed."));
						}
						return;
					}

					if (!ThisPtr->WasConstructSuccessful())
					{
						if (OnComplete)
						{
							OnComplete(false, TEXT("Construct was not called."));
						}
						return;
					}

					if (!TabbedWidgetProps.IsValid())
					{
						if (OnComplete)
						{
							OnComplete(false, TEXT("Relevant tabbed widget is invalid."));
						}
						return;
					}

					if (!TabbedWidgetProps->TabAssetBP.IsValid())
					{
						if (OnComplete)
						{
							OnComplete(false, TEXT("Relevant tabbed widget class is invalid."));
						}
						return;
					}

					if (bIsMainWidget)
					{
						for (int32 i = ThisPtr->AdditionalUIWidgetsProps.Num() - 1; i >= 0; i--)
						{
							ThisPtr->UnregisterAdditionalTab(ThisPtr->AdditionalUIWidgetsProps[i]);
						}
					}
					else
					{
						ThisPtr->UnregisterAdditionalTab(TabbedWidgetProps);
					}

					ThisPtr->RegisterTab_Internal(bIsMainWidget, TabbedWidgetProps);

					if (OnComplete)
					{
						OnComplete(true, TEXT(""));
					}
				});
			EdWorld->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 0.01f, false);
		}
	}
}

void UBKViewManager::RegisterTab_Internal(bool bIsMainWidget, TSharedPtr<TabbedUIWidgetProperties> TabbedWidgetProps)
{
	UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();

	EditorUtilitySubsystem->SpawnAndRegisterTab(TabbedWidgetProps->TabAssetBP.Get());

	FLevelEditorModule& LevelEditor = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
	TSharedPtr<FTabManager> LevelEditorTabManager = LevelEditor.GetLevelEditorTabManager();

	TSharedPtr<SDockTab> CreatedTab = LevelEditorTabManager->FindExistingLiveTab(TabbedWidgetProps->TabRegistrationName);
	ensure (CreatedTab.IsValid());

	TabbedWidgetProps->TabAssetBPLastProcessedWidget = TabbedWidgetProps->TabAssetBP->GetCreatedWidget();
	TabbedWidgetProps->TabWeakPtr = CreatedTab;
	//CreatedTab->SetTabIcon(FSlateIcon(StyleSet->GetStyleSetName(), "BMainTabIcon", "BMainTabIcon.Small").GetSmallIcon());
	CreatedTab->SetLabel(FText::FromString(TabbedWidgetProps->TabTitle));
	CreatedTab->SetContentScale(FVector2D(1.08f, 1.92f));
	CreatedTab->SetOnTabClosed(SDockTab::FOnTabClosedCallback::CreateUObject(this, &UBKViewManager::OnTabBeingClosed));
	CreatedTab->FlashTab();

	if (IBlutilityModule* BlutilityModule = FModuleManager::GetModulePtr<IBlutilityModule>("Blutility"))
	{
		BlutilityModule->RemoveLoadedScriptUI(TabbedWidgetProps->TabAssetBP.Get()); //We do not need it.
	}

	if (bIsMainWidget)
	{
		SelectMainViewAndTryValidatingUser();
	}
	else
	{
		AdditionalUIWidgetsProps.Add(TabbedWidgetProps);
	}
}

void UBKViewManager::UnregisterMainTab()
{
	Unregister_Internal(BaseUIWidgetProps);
}

void UBKViewManager::UnregisterAdditionalTab(TSharedPtr<TabbedUIWidgetProperties> TabbedWidgetProps)
{
	if (!TabbedWidgetProps.IsValid()) return;

	bool bWasContained = false;

	//This must be done before RemoveTabFromParent
	for (int32 i = 0; i < AdditionalUIWidgetsProps.Num(); i++)
	{
		if (AdditionalUIWidgetsProps[i].IsValid())
		{
			if (AdditionalUIWidgetsProps[i] == TabbedWidgetProps)
			{
				bWasContained = true;
				AdditionalUIWidgetsProps.RemoveAt(i);
				break;
			}
		}
	}

	//If that is already unregistered; no need to re-run.
	if (!bWasContained) return;

	if (TabbedWidgetProps->TabWeakPtr.IsValid())
	{
		//This function triggers OnTabBeingClosed!
		TabbedWidgetProps->TabWeakPtr.Pin()->RemoveTabFromParent();
	}

	Unregister_Internal(TabbedWidgetProps);
}

void UBKViewManager::Unregister_Internal(TSharedPtr<TabbedUIWidgetProperties> TabbedWidgetProps)
{
	if (TabbedWidgetProps->TabAssetBP.IsValid())
	{
		if (IBlutilityModule* BlutilityModule = FModuleManager::GetModulePtr<IBlutilityModule>("Blutility"))
		{
			BlutilityModule->RemoveLoadedScriptUI(TabbedWidgetProps->TabAssetBP.Get());
		}
	}
	if (FLevelEditorModule* LevelEditor = FModuleManager::GetModulePtr<FLevelEditorModule>("LevelEditor"))
	{
		TSharedPtr<FTabManager> LevelEditorTabManager = LevelEditor->GetLevelEditorTabManager();
		if (LevelEditorTabManager.IsValid())
		{
			while (LevelEditorTabManager->UnregisterTabSpawner(TabbedWidgetProps->TabRegistrationName)) {}
		}
	}
}

void UBKViewManager::OnTabBeingClosed(TSharedRef<SDockTab> TabBeingClosed)
{
	if (BaseUIWidgetProps.IsValid() && TabBeingClosed == BaseUIWidgetProps->TabWeakPtr)
	{
		if (BaseUIWidgetProps->TabAssetBPLastProcessedWidget.IsValid())
		{
			((UBKViewWidget*)BaseUIWidgetProps->TabAssetBPLastProcessedWidget.Get())->OnTabBeingClosed();
		}

		if (UBKNetworkManager* NetworkManager = UBKManagerStore::GetNetworkManager())
		{
			auto ObserverWidgetsCopy = NetworkManager->GetObserverWidgetsCopy();
			for (auto Current : ObserverWidgetsCopy)
			{
				if (Current.IsValid())
				{
					Current->OnTabBeingClosed();
				}
			}
		}

		RegisterTab(true, BaseUIWidgetProps);
	}
	else
	{
		//Note-Todo: Calling OnTabBeingClosed() may not be sufficient, because there could be other authenticated widgets owned by the widget; still bound.
		for (auto Current : AdditionalUIWidgetsProps)
		{
			if (Current.IsValid())
			{
				if (Current->TabWeakPtr == TabBeingClosed)
				{
					if (Current->TabAssetBPLastProcessedWidget.IsValid())
					{
						((UBKViewWidget*)Current->TabAssetBPLastProcessedWidget.Get())->OnTabBeingClosed();
					}
					//This will call OnTabBeingClosed again, but since it removes from the list first; there is no way of infinite loop occurence.
					UnregisterAdditionalTab(Current);
					return;
				}
			}
		}
	}
}

void UBKViewManager::Tick(float DeltaTime)
{
	FDisplayMetrics DisplayMetrics;
	FSlateApplication::Get().GetInitialDisplayMetrics(DisplayMetrics);
	FVector2D WindowSize(DisplayMetrics.PrimaryDisplayHeight / 3, DisplayMetrics.PrimaryDisplayWidth / 3);

	if (BaseUIWidgetProps.IsValid())
	{
		EnsureTabSizeInTick(BaseUIWidgetProps->TabWeakPtr, WindowSize);
	}
	for (auto Current : AdditionalUIWidgetsProps)
	{
		if (Current.IsValid())
		{
			EnsureTabSizeInTick(Current->TabWeakPtr, WindowSize);
		}
	}

	ValidateUserRetryTimer += DeltaTime;
	if (ValidateUserRetryTimer >= VALIDATE_RETRY_TIME)
	{
		ValidateUserRetryTimer = 0.0f;
		if (BaseUIWidgetProps.IsValid() && BaseUIWidgetProps->TabAssetBPLastProcessedWidget.IsValid())
		{
			if (UBKNetworkManager* NetworkManager = UBKManagerStore::GetNetworkManager())
			{
				NetworkManager->ValidateUser(true, BKCancellationTokenWeakWrapper::MakeFromWidget(BaseUIWidgetProps->TabAssetBPLastProcessedWidget.Get()));
			}
		}
	}
}

void UBKViewManager::EnsureTabSizeInTick(TWeakPtr<SDockTab> TabWeakPtr, const FVector2D& WindowSize)
{
	if (TabWeakPtr.IsValid())
	{
		TSharedPtr<SWindow> ParentWindow = TabWeakPtr.Pin()->GetParentWindow();
		if (ParentWindow.IsValid())
		{
			//Here; tab is not being dragged and not docked somewhere.

			ParentWindow->Resize(WindowSize);

			if (!ParentWindow->IsModalWindow())
			{
				ParentWindow->SetAsModalWindow();
				ParentWindow->SetSizingRule(ESizingRule::FixedSize);
				ParentWindow->SetNativeWindowButtonsVisibility(false);
				ParentWindow->SetViewportSizeDrivenByWindow(false);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
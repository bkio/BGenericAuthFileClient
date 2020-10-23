/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "UI/BWidget.h"
#include "Engine/Engine.h"
#include "Managers/BManagerStore.h"
#include "Managers/BViewManager.h"
#include "Managers/BNetworkManager.h"

void UBKViewWidget::OnAuthenticationStatusChanged(bool bNewAuthenticationStatus)
{
	uint8 EnumNewStatus = bNewAuthenticationStatus ? 1 : 0;
	if (bFocusIsCurrentView && LastProcessedAuthenticationStatus != EnumNewStatus)
	{
		LastProcessedAuthenticationStatus = EnumNewStatus;
		AuthenticationStatusChangedAndFocusViewIsThis(bNewAuthenticationStatus);
	}
}

void UBKViewWidget::OnViewChanged_Internal(int32 ViewIndex)
{
	if (ViewIndex == 0) return;

	bFocusIsCurrentView = ViewIndex == (int32)WidgetViewType;
	if (bFocusIsCurrentView)
	{
		if (UBKNetworkManager* NetworkManager = UBKManagerStore::GetNetworkManager())
			NetworkManager->RegisterAuthenticatedWidgetObserver(this);

		FocusViewIsThisWidget();
	}
	else
	{
		if (UBKNetworkManager* NetworkManager = UBKManagerStore::GetNetworkManager())
			NetworkManager->UnregisterAuthenticatedWidgetObserver(this);

		LastProcessedAuthenticationStatus = 0;
		FocusViewIsAnotherWidget();
	}
}

void UBKViewWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (WidgetViewType == EViewType::None)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Variable WidgetViewType must be set in %s"), *GetName()));
		return;
	}
	if (WidgetAuthenticatable == EWidgetAuthenticatable::NotSet)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Variable WidgetAuthenticatable must be set in %s"), *GetName()));
		return;
	}

	if (UBKViewManager* ViewManager = UBKManagerStore::GetViewManager())
	{
		ViewManager->OnViewChanged.AddDynamic(this, &UBKViewWidget::OnViewChanged_Internal);
		OnViewChanged_Internal((int32)ViewManager->GetCurrentView());
	}
}

void UBKViewWidget::NativeDestruct()
{
	InvalidateWidgetBounds();
	Super::NativeDestruct();
}

void UBKViewWidget::OnOwnerBlueprintCompiled()
{
	InvalidateWidgetBounds();
}

void UBKViewWidget::OnTabBeingClosed()
{
	InvalidateWidgetBounds();
}

void UBKViewWidget::InvalidateWidgetBounds()
{
	if (UBKNetworkManager* NetworkManager = UBKManagerStore::GetNetworkManager())
		NetworkManager->UnregisterAuthenticatedWidgetObserver(this);

	if (UBKViewManager* ViewManager = UBKManagerStore::GetViewManager())
		ViewManager->OnViewChanged.RemoveDynamic(this, &UBKViewWidget::OnViewChanged_Internal);
}
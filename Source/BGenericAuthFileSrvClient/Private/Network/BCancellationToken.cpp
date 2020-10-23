/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "Network/BCancellationToken.h"
#include "Managers/BManagerStore.h"
#include "Managers/BNetworkManager.h"
#include "Components/Widget.h"
#include "Widgets/SWidget.h"

UBKCancellationToken* UBKCancellationToken::MakeFromUObject(UObject* Object)
{
	UBKCancellationToken* NewToken = NewObject<UBKCancellationToken>(UBKManagerStore::GetNetworkManager());
	NewToken->Data = BKCancellationTokenWeakWrapper::MakeFromUObject(Object);
	return NewToken;
}

UBKCancellationToken* UBKCancellationToken::MakeFromWidget(UWidget* Widget)
{
	UBKCancellationToken* NewToken = NewObject<UBKCancellationToken>(UBKManagerStore::GetNetworkManager());
	NewToken->Data = BKCancellationTokenWeakWrapper::MakeFromWidget(Widget);
	return NewToken;
}

bool UBKCancellationToken::IsTokenValid()
{
	if (!Data.IsValid()) return false;
	return Data->IsTokenValid();
}

void UBKCancellationToken::ForceKill()
{
	if (Data.IsValid())
		Data->ForceKill();
}

TSharedPtr<BKCancellationTokenWeakWrapper> UBKCancellationToken::GetData() const
{
	return Data;
}

BKCancellationTokenWeakWrapper::BKCancellationTokenWeakWrapper()
{
	MadeFrom = ECancellationTokenMadeFrom::MadeFrom_None;
}

TSharedPtr<BKCancellationTokenWeakWrapper> BKCancellationTokenWeakWrapper::MakeFromUObject(UObject* Object)
{
	TSharedPtr<BKCancellationTokenWeakWrapper> NewWrapper = MakeShareable<BKCancellationTokenWeakWrapper>(new BKCancellationTokenWeakWrapper);
	if (Object && Object->IsValidLowLevel() && !Object->IsPendingKillOrUnreachable())
	{
		NewWrapper->MadeFrom = ECancellationTokenMadeFrom::MadeFrom_UObject;
		NewWrapper->MadeFrom_UObject = Object;
	}
	else
	{
		NewWrapper->MadeFrom = ECancellationTokenMadeFrom::MadeFrom_None;
	}
	return NewWrapper;
}

TSharedPtr<BKCancellationTokenWeakWrapper> BKCancellationTokenWeakWrapper::MakeFromWidget(UWidget* Widget)
{
	TSharedPtr<BKCancellationTokenWeakWrapper> NewWrapper = MakeShareable<BKCancellationTokenWeakWrapper>(new BKCancellationTokenWeakWrapper);
	if (Widget && Widget->IsValidLowLevel() && !Widget->IsPendingKillOrUnreachable())
	{
		NewWrapper->MadeFrom = ECancellationTokenMadeFrom::MadeFrom_UWidget;
		NewWrapper->MadeFrom_UWidget = Widget->GetCachedWidget();
		if (!NewWrapper->MadeFrom_UWidget.IsValid())
		{
			NewWrapper->MadeFrom = ECancellationTokenMadeFrom::MadeFrom_None;
		}
	}
	else
	{
		NewWrapper->MadeFrom = ECancellationTokenMadeFrom::MadeFrom_None;
	}
	return NewWrapper;

}

bool BKCancellationTokenWeakWrapper::IsTokenValid()
{
	if (bForcedKill) return false;

	switch (MadeFrom)
	{
	case ECancellationTokenMadeFrom::MadeFrom_UObject:
		return MadeFrom_UObject.IsValid();
	case ECancellationTokenMadeFrom::MadeFrom_UWidget:
		return MadeFrom_UWidget.IsValid();
	default:
		return false;
	}
}

void BKCancellationTokenWeakWrapper::ForceKill()
{
	bForcedKill = true;
}

bool BKCancellationTokenWeakWrapper::Equals(TSharedPtr<BKCancellationTokenWeakWrapper> OtherCancellationToken)
{
	if (OtherCancellationToken.IsValid())
	{
		return 
			OtherCancellationToken->bForcedKill == bForcedKill
			&& OtherCancellationToken->MadeFrom == MadeFrom
			&& OtherCancellationToken->MadeFrom_UObject == MadeFrom_UObject
			&& OtherCancellationToken->MadeFrom_UWidget == MadeFrom_UWidget;
	}
	return false;
}
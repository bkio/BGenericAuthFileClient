/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "BCancellationToken.generated.h"

enum ECancellationTokenMadeFrom
{
	MadeFrom_None,
	MadeFrom_UObject,
	MadeFrom_UWidget
};

class BKCancellationTokenWeakWrapper
{

private:
	BKCancellationTokenWeakWrapper();

	ECancellationTokenMadeFrom MadeFrom;

	TWeakObjectPtr<UObject> MadeFrom_UObject;
	TWeakPtr<class SWidget> MadeFrom_UWidget;

	bool bForcedKill = false;

public:
	bool Equals(TSharedPtr<BKCancellationTokenWeakWrapper> OtherCancellationToken);

	static TSharedPtr<BKCancellationTokenWeakWrapper> MakeFromUObject(UObject* Object);
	static TSharedPtr<BKCancellationTokenWeakWrapper> MakeFromWidget(class UWidget* Widget);
	
	bool IsTokenValid();
	void ForceKill();
};

UCLASS(BlueprintType)
class BGENERICAUTHFILESRVCLIENT_API UBKCancellationToken : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "BGenericAuthFileSrvClient|Network")
	static UBKCancellationToken* MakeFromUObject(UObject* Object);

	UFUNCTION(BlueprintPure, Category = "BGenericAuthFileSrvClient|Network")
	static UBKCancellationToken* MakeFromWidget(class UWidget* Widget);

	UFUNCTION(BlueprintCallable, Category = "BGenericAuthFileSrvClient|Network")
	bool IsTokenValid();

	UFUNCTION(BlueprintCallable, Category = "BGenericAuthFileSrvClient|Network")
	void ForceKill();

	TSharedPtr<BKCancellationTokenWeakWrapper> GetData() const;

private:
	TSharedPtr<BKCancellationTokenWeakWrapper> Data;
};
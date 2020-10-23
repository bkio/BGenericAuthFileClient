/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "CoreMinimal.h"
#include "Managers/BManager.h"
#include "BFileOperationManager.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class BGENERICAUTHFILESRVCLIENT_API UBKFileOperationManager : public UBKManager
{
	GENERATED_BODY()
	
protected:
	virtual void OnConstruct() override;
	virtual void OnDestruct() override;

	virtual void OnFirstTimeMapLoaded() override;
	virtual void OnNonFirstTimeMapLoaded() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "BGenericAuthFileSrvClient|Managers|File Operation Manager")
	void OnBTick(float DeltaTime);

private:
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;

	bool bIsTickable = false;

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "BGenericAuthFileSrvClient|Managers|File Operation Manager")
	void OnConstructBP();

	UFUNCTION(BlueprintImplementableEvent, Category = "BGenericAuthFileSrvClient|Managers|File Operation Manager")
	void OnDestructBP();

	UFUNCTION(BlueprintImplementableEvent, Category = "BGenericAuthFileSrvClient|Managers|File Operation Manager")
	void OnFirstTimeMapLoadedBP();

	UFUNCTION(BlueprintImplementableEvent, Category = "BGenericAuthFileSrvClient|Managers|File Operation Manager")
	void OnNonFirstTimeMapLoadedBP();

	UFUNCTION(BlueprintCallable, Category = "BGenericAuthFileSrvClient|Managers|File Operation Manager")
	void SetIsTickable(bool bTickable);
};
/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "CoreMinimal.h"
#include "Managers/BManager.h"
#include "BActionManager.generated.h"

/**
 * 
 */
UCLASS()
class BGENERICAUTHFILESRVCLIENT_API UBKActionManager : public UBKManager
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "BGenericAuthFileSrvClient|Actions")
	void Initialize(bool _bExecuteOneActionPerTick = false);

	UFUNCTION(BlueprintCallable, Category = "BGenericAuthFileSrvClient|Actions")
	void QueueAction(class UBKActionBase* Action);

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void OnConstruct() override;
	virtual void OnDestruct() override;

private:
	virtual void OnMapIsBeingTornDown() override;

	UFUNCTION()
	void ExecuteAllActions();
	UFUNCTION()
	void ExecuteNextAction();
	UFUNCTION()
	void CleanUpFinishedActions();

	UPROPERTY()
	TArray<class UBKActionBase*> ActionsToExecute;
	UPROPERTY()
	TArray<class UBKActionBase*> ActionsAwaitingResponse;
	UPROPERTY()
	bool bExecuteOneActionPerTick = false;
};
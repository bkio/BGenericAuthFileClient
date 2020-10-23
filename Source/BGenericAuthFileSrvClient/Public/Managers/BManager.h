/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TickableEditorObject.h"
#include "BManager.generated.h"

enum class EMapChangeType : uint8;

UCLASS()
class BGENERICAUTHFILESRVCLIENT_API UBKManager : public UObject, public FTickableEditorObject
{
	GENERATED_BODY()
	
protected:
	virtual void OnConstruct();
	virtual void OnDestruct();

	virtual void OnFirstTimeMapLoaded() {}
	virtual void OnNonFirstTimeMapLoaded() {}
	virtual void OnMapIsBeingTornDown() {}

	bool WasConstructSuccessful();

	virtual void Tick(float DeltaTime) override {}

private:
	void OnFirstTimeMapLoaded_Internal(class UWorld* World, EMapChangeType MapChangeType);
	void OnNonFirstTimeMapLoaded_Internal(class UWorld* World, EMapChangeType MapChangeType);

	bool bConstructSuccessful = false;

	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;

public:
	friend class UBKManagerStore; //This should have call access to OnConstruct and OnDestruct; also by protected's nature; child managers will have too but it is not intended by design.

	static FString GenerateRandomANSIString();
};
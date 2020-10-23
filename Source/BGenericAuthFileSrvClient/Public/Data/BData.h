/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "CoreMinimal.h"
#include "BData.generated.h"

USTRUCT()
struct BGENERICAUTHFILESRVCLIENT_API FBKDataStruct
{
	GENERATED_USTRUCT_BODY()

	virtual void PruneForUpdateCall(TSharedPtr<class FJsonObject> CompiledDataJsonObject) {}
	virtual ~FBKDataStruct() {}
};
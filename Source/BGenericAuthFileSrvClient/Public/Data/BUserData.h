/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "CoreMinimal.h"
#include "Data/BData.h"
#include "BUserData.generated.h"

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FAuthMethod : public FBKDataStruct
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	FString method;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	FString apiKey;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	FString userEmail;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	FString userName;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	FString passwordMd5;
};

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FAccessScope : public FBKDataStruct
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	FString wildcardPath;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	TArray<FString> accessRights;
};

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FUser : public FBKDataStruct
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	FString userName;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	FString userEmail;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	TArray<FAuthMethod> authMethods;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	TArray<FAccessScope> baseAccessScope;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	TArray<FString> userModels;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	TArray<FString> userSharedModels;

	virtual void PruneForUpdateCall(TSharedPtr<class FJsonObject> CompiledDataJsonObject) override {}
};
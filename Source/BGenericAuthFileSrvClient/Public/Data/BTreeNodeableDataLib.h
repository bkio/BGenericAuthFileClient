/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "CoreMinimal.h"
#include "Data/BModelData.h"
#include "BTreeNodeableDataLib.generated.h"

USTRUCT(BlueprintType)
struct FBKNodeElementValues
{
	GENERATED_USTRUCT_BODY()

public:
	FBKNodeElementValues() {};
	FBKNodeElementValues(const TArray<FString>& Values)
	{
		this->Values = Values;
	};
	FBKNodeElementValues(const FString& Value)
	{
		Values.Add(Value);
	};

	UPROPERTY(BlueprintReadWrite)
	TArray<FString> Values;
};

UCLASS()
class BGENERICAUTHFILESRVCLIENT_API UBKTreeNodeableDataLib : public UObject
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, DisplayName = "Get As Node Element Values", Category = "BGenericAuthFileSrvClient|Actions|Data")
	static TMap<FString, FBKNodeElementValues> GetAsNodeElementValues_1(const FVersionFileEntry& Input);
	UFUNCTION(BlueprintCallable, DisplayName = "Get As Node Element Values", Category = "BGenericAuthFileSrvClient|Actions|Data")
	static TMap<FString, FBKNodeElementValues> GetAsNodeElementValues_2(const FModelRevisionVersion& Input);
	UFUNCTION(BlueprintCallable, DisplayName = "Get As Node Element Values", Category = "BGenericAuthFileSrvClient|Actions|Data")
	static TMap<FString, FBKNodeElementValues> GetAsNodeElementValues_3(const FModelRevision& Input);
	UFUNCTION(BlueprintCallable, DisplayName = "Get As Node Element Values", Category = "BGenericAuthFileSrvClient|Actions|Data")
	static TMap<FString, FBKNodeElementValues> GetAsNodeElementValues_4(const FModelMetadata& Input);
	UFUNCTION(BlueprintCallable, DisplayName = "Get As Node Element Values", Category = "BGenericAuthFileSrvClient|Actions|Data")
	static TMap<FString, FBKNodeElementValues> GetAsNodeElementValues_5(const FModelInfo& Input);
};
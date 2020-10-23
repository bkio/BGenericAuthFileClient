/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Data/BModelData.h"
#include "BActionBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class BGENERICAUTHFILESRVCLIENT_API UBKActionBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	/*
	* Common methods for download file actions
	*/
	static void Execute_DownloadFile_Action(
		class UBKActionBase* Action,
		const FString& UrlPostFix,
		const FString& DestinationPath,
		const FString& ModelId,
		int32 RevisionIndex,
		int32 VersionIndex,
		const TFunction<void(float)>& ProgressCallback,
		const TFunction<void(const FString&, int32)>& FailureCallback,
		const TFunction<void(const FString&)>& SuccessCallback);

	UFUNCTION(BlueprintPure, Category = "BGenericAuthFileSrvClient|Utilities")
	static UPARAM(DisplayName = "FileId") FString MakeFileId(const FString& ModelId, const int32& RevisionIndex, const int32& VersionIndex);
};
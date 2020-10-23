/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "CoreMinimal.h"
#include "Data/BData.h"
#include "BModelData.generated.h"

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FVersionFileEntry : public FBKDataStruct
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	FString fileEntryName;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	FString fileEntryFileType;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	FString zipMainAssemblyFileNameIfAny;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	TArray<FString> fileEntryComments;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	FString fileEntryCreationTime;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	FString rawFileRelativeUrl;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	int32 fileProcessStage;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	FString fileProcessedAtTime;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	int32 processedFilesRootNodeId;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	FString hierarchyRAFRelativeUrl;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	FString hierarchyCFRelativeUrl;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	FString geometryRAFRelativeUrl;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	FString geometryCFRelativeUrl;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	FString metadataRAFRelativeUrl;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	FString metadataCFRelativeUrl;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	bool generateUploadUrl;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	FString dataSource;

	//Both upload raw file and update file entry calls use POST requests; generateUploadUrl true/false and provided fields make the difference.
	virtual void PruneForUpdateCall(TSharedPtr<class FJsonObject> CompiledDataJsonObject) override; //Common prune for PruneForUploadCall and PruneForPostCall
	void PruneForUploadCall(TSharedPtr<class FJsonObject> CompiledDataJsonObject); //To get upload url for new upload and changing some new file related fields
	void PruneForPostCall(TSharedPtr<class FJsonObject> CompiledDataJsonObject); //To update the file entry fields (irrelevant to the file itself)
};
//Not FVersionFileEntry properties, but being sent in responses
#define FILE_DOWNLOAD_URL_PROPERTY FString(TEXT("fileDownloadUrl"))
#define FILE_UPLOAD_URL_PROPERTY FString(TEXT("fileUploadUrl"))
#define FILE_UPLOAD_CONTENT_TYPE_PROPERTY FString(TEXT("fileUploadContentType"))
#define FILE_DOWNLOAD_UPLOAD_EXPIRY_MINUTES_PROPERTY FString(TEXT("expiryMinutes"))

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FModelRevisionVersion : public FBKDataStruct
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	int32 versionIndex;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	FString versionName;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	TArray<FString> versionComments;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	FString versionCreationTime;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	FVersionFileEntry versionFileEntry;

	virtual void PruneForUpdateCall(TSharedPtr<class FJsonObject> CompiledDataJsonObject) override;
};

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FModelRevision : public FBKDataStruct
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	int32 revisionIndex;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	FString revisionName;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	TArray<FString> revisionComments;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	FString revisionCreationTime;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	TArray<FModelRevisionVersion> revisionVersions;

	virtual void PruneForUpdateCall(TSharedPtr<class FJsonObject> CompiledDataJsonObject) override;
};

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FModelMetadata : public FBKDataStruct
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	FString metadataKey;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	TArray<FString> metadataValues;

	virtual void PruneForUpdateCall(TSharedPtr<class FJsonObject> CompiledDataJsonObject) override;
};

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FModelInfo : public FBKDataStruct
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	FString modelOwnerUserId;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	FString modelId;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	FString modelUniqueName;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	TArray<FString> modelComments;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	TArray<FModelRevision> modelRevisions;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	TArray<FModelMetadata> modelMetadata;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	TArray<FString> modelSharedWithUserIds;

	virtual void PruneForUpdateCall(TSharedPtr<class FJsonObject> CompiledDataJsonObject) override;
};


USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FShareInfo : public FBKDataStruct
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
		bool shareWithAll;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
		TArray<FString> userIds;

	virtual void PruneForUpdateCall(TSharedPtr<class FJsonObject> CompiledDataJsonObject) override;
};

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FUserEmailAddress : public FBKDataStruct
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
		FString userId;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
		FString userEmail;

};

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FRegisteredEmailAddresses : public FBKDataStruct
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
		TArray<FUserEmailAddress> emailAddresses;

};


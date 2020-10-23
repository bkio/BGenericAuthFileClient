/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "CoreMinimal.h"
#include "Data/BData.h"
#include "BProcessData.generated.h"

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FJVector2D : public FBKDataStruct
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	float x;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	float y;

	virtual void PruneForUpdateCall(TSharedPtr<class FJsonObject> CompiledDataJsonObject) override {}
};

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FJVector3D : public FBKDataStruct
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	float x;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	float y;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	float z;

	virtual void PruneForUpdateCall(TSharedPtr<class FJsonObject> CompiledDataJsonObject) override {}
};

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FJNormalTangent : public FBKDataStruct
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	FJVector3D normal;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	FJVector3D tangent;

	virtual void PruneForUpdateCall(TSharedPtr<class FJsonObject> CompiledDataJsonObject) override {}
};

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FJGeometryPart : public FBKDataStruct
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	int64 geometryId;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	FJVector3D location;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	FJVector3D rotation;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	FJVector3D scale;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	FJVector3D color;

	virtual void PruneForUpdateCall(TSharedPtr<class FJsonObject> CompiledDataJsonObject) override {}
};

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FJHierarchyNode : public FBKDataStruct
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	int64 parentId;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	int64 metadataId;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	TArray<FJGeometryPart> geometryParts;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	TArray<int64> childNodes;

	virtual void PruneForUpdateCall(TSharedPtr<class FJsonObject> CompiledDataJsonObject) override {}
};

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FJGeometryNode : public FBKDataStruct
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	TArray<FJVector3D> vertexes;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	TArray<int64> indexes;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	TArray<FJNormalTangent> normalTangentList;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	TArray<FJVector2D> uvList;

	virtual void PruneForUpdateCall(TSharedPtr<class FJsonObject> CompiledDataJsonObject) override {}
};

USTRUCT(BlueprintType)
struct BGENERICAUTHFILESRVCLIENT_API FJMetadataNode : public FBKDataStruct
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BGenericAuthFileSrvClient|Actions|Data")
	FString metadata;

	virtual void PruneForUpdateCall(TSharedPtr<class FJsonObject> CompiledDataJsonObject) override {}
};

enum EBFormatStructure
{
	TopAssemblyDriven,
	Homogeneous
};

struct FBFormat
{

private:
	FBFormat() {}

public:
	FString Description;
	EBFormatStructure StructureDefinition;
	FString TopAssemblyFormat;

	FBFormat(const FString& InDescription, const FString& InTopAssemblyFormat)
	{
		Description = InDescription;
		StructureDefinition = EBFormatStructure::TopAssemblyDriven;
		TopAssemblyFormat = InTopAssemblyFormat;
	}
	FBFormat(const FString& InDescription)
	{
		Description = InDescription;
		StructureDefinition = EBFormatStructure::Homogeneous;
	}
};

class BSupportedFileFormats
{

private:
	BSupportedFileFormats();

	void CreateFormatToDescriptionMap();
	void CompileDescriptionToFormatsMap();
	void CompileGroups();
	void CompileForDelimited();
	void CompileFileDialogFriendly();
	void CompileTopAssembliesFileDialogFriendly();

	TMap<FString, FBFormat> F2D;
	TMap<FString, TArray<FString>> D2F;
	TMap<FString, TArray<FString>> F2Groups;
	TMap<FString, TArray<FString>> GroupToFormats;
	FString FormatsDelimited;
	FString FormatsFileDialogFriendly;
	TMap<FString, FString> FormatToTopAssemblyFileDialogFriendly;
	TMap<FString, FString> GroupToTopAssemblyFileDialogFriendly;

public:
	static BSupportedFileFormats& Get()
	{
		static BSupportedFileFormats Instance;
		return Instance;
	}

	const FString& GetFormatsFileDialogFriendly();
	
	void FilterOutUnsupported(TArray<FString>& FilePaths);
	void FilterOutFilesExceptGivenGroups(TArray<FString>& FilePaths, const TArray<FString>& Groups);

	TArray<FString> GetGroupFromFiles(const TArray<FString>& FilePaths);
	bool TryGetTopAssemblyFileDialogFriendlyForFileFormat(const FString& FileFormat, FString& TopAssemblyFileDialogFriendly);
	bool TryGetTopAssemblyFileDialogFriendlyForGroup(const FString& Group, FString& TopAssemblyFileDialogFriendly);
};
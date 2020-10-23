/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "Data/BTreeNodeableDataLib.h"

TMap<FString, FBKNodeElementValues>  UBKTreeNodeableDataLib::GetAsNodeElementValues_1(const FVersionFileEntry& Input)
{
	TMap<FString, FBKNodeElementValues> ReturnValue;
	ReturnValue.Add("Comments", FBKNodeElementValues(Input.fileEntryComments));
	ReturnValue.Add("CreationTime", FBKNodeElementValues(Input.fileEntryCreationTime));
	ReturnValue.Add("Name", FBKNodeElementValues(Input.fileEntryName));
	ReturnValue.Add("DataSource", FBKNodeElementValues(Input.dataSource));
	ReturnValue.Add("FileType", FBKNodeElementValues(Input.fileEntryFileType));
	ReturnValue.Add("ProcessedAtTime", FBKNodeElementValues(Input.fileProcessedAtTime));
	ReturnValue.Add("ProcessStage", FBKNodeElementValues(FString::FromInt(Input.fileProcessStage)));
	ReturnValue.Add("ZipAssemblyFileName", FBKNodeElementValues(Input.zipMainAssemblyFileNameIfAny));
	return ReturnValue;
}

TMap<FString, FBKNodeElementValues> UBKTreeNodeableDataLib::GetAsNodeElementValues_2(const FModelRevisionVersion& Input)
{
	TMap<FString, FBKNodeElementValues> ReturnValue;
	ReturnValue.Add("Name", FBKNodeElementValues(Input.versionName));
	ReturnValue.Add("Comments", FBKNodeElementValues(Input.versionComments));
	ReturnValue.Add("Index", FBKNodeElementValues(FString::FromInt(Input.versionIndex)));
	ReturnValue.Add("CreationTime", FBKNodeElementValues(Input.versionCreationTime));
	return ReturnValue;
}

TMap<FString, FBKNodeElementValues> UBKTreeNodeableDataLib::GetAsNodeElementValues_3(const FModelRevision& Input)
{
	TMap<FString, FBKNodeElementValues> ReturnValue;
	ReturnValue.Add("Name", FBKNodeElementValues(Input.revisionName));
	ReturnValue.Add("Comments", FBKNodeElementValues(Input.revisionComments));
	ReturnValue.Add("Index", FBKNodeElementValues(FString::FromInt(Input.revisionIndex)));
	ReturnValue.Add("CreationTime", FBKNodeElementValues(Input.revisionCreationTime));
	return ReturnValue;
}

TMap<FString, FBKNodeElementValues> UBKTreeNodeableDataLib::GetAsNodeElementValues_4(const FModelMetadata& Input)
{
	TMap<FString, FBKNodeElementValues> ReturnValue;
	ReturnValue.Add("metadataKey", FBKNodeElementValues(Input.metadataKey));
	ReturnValue.Add("metadataValues", FBKNodeElementValues(Input.metadataValues));
	return ReturnValue;
}

TMap<FString, FBKNodeElementValues> UBKTreeNodeableDataLib::GetAsNodeElementValues_5(const FModelInfo& Input)
{
	TMap<FString, FBKNodeElementValues> ReturnValue;
	ReturnValue.Add("Name", FBKNodeElementValues(Input.modelUniqueName));
	ReturnValue.Add("Id", FBKNodeElementValues(Input.modelId));
	ReturnValue.Add("Comments", FBKNodeElementValues(Input.modelComments));
	ReturnValue.Add("SharedWithUserIds", FBKNodeElementValues(Input.modelSharedWithUserIds));
	ReturnValue.Add("OwnerId", FBKNodeElementValues(Input.modelOwnerUserId));
	return ReturnValue;
}
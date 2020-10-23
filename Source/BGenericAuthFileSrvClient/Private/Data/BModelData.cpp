/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "Data/BModelData.h"
#include "JsonUtilities.h"

void FVersionFileEntry::PruneForUpdateCall(TSharedPtr<FJsonObject> CompiledDataJsonObject)
{
	/*
	All except

	fileEntryName,
	fileEntryFileType,
	fileEntryComments,
	zipMainAssemblyFileNameIfAny,
	generateUploadUrl,
	dataSource
	*/
	CompiledDataJsonObject->RemoveField("fileEntryCreationTime");
	CompiledDataJsonObject->RemoveField("rawFileRelativeUrl");
	CompiledDataJsonObject->RemoveField("fileProcessStage");
	CompiledDataJsonObject->RemoveField("fileProcessedAtTime");
	CompiledDataJsonObject->RemoveField("processedFilesRootNodeId");
	CompiledDataJsonObject->RemoveField("hierarchyRAFRelativeUrl");
	CompiledDataJsonObject->RemoveField("hierarchyCFRelativeUrl");
	CompiledDataJsonObject->RemoveField("geometryRAFRelativeUrl");
	CompiledDataJsonObject->RemoveField("geometryCFRelativeUrl");
	CompiledDataJsonObject->RemoveField("metadataRAFRelativeUrl");
	CompiledDataJsonObject->RemoveField("metadataCFRelativeUrl");
	if (CompiledDataJsonObject->GetStringField("zipMainAssemblyFileNameIfAny").IsEmpty())
		CompiledDataJsonObject->RemoveField("zipMainAssemblyFileNameIfAny");
}

void FVersionFileEntry::PruneForUploadCall(TSharedPtr<FJsonObject> CompiledDataJsonObject)
{
	/*
	Expected only:

	generateUploadUrl = true
	fileEntryName
	fileEntryFileType
	zipMainAssemblyFileNameIfAny
	dataSource
	*/
	PruneForUpdateCall(CompiledDataJsonObject);
	CompiledDataJsonObject->RemoveField("fileEntryComments");
	CompiledDataJsonObject->SetBoolField("generateUploadUrl", true);
}
void FVersionFileEntry::PruneForPostCall(TSharedPtr<FJsonObject> CompiledDataJsonObject)
{
	/*
	Expected only:

	generateUploadUrl = false
	fileEntryComments
	*/
	PruneForUpdateCall(CompiledDataJsonObject);
	CompiledDataJsonObject->RemoveField("fileEntryName");
	CompiledDataJsonObject->RemoveField("fileEntryFileType");
	CompiledDataJsonObject->RemoveField("zipMainAssemblyFileNameIfAny");
	CompiledDataJsonObject->RemoveField("dataSource");
	CompiledDataJsonObject->SetBoolField("generateUploadUrl", false);
}

void FModelRevisionVersion::PruneForUpdateCall(TSharedPtr<FJsonObject> CompiledDataJsonObject)
{
	CompiledDataJsonObject->RemoveField("versionIndex");
	CompiledDataJsonObject->RemoveField("versionCreationTime");
	CompiledDataJsonObject->RemoveField("versionFileEntry");
}

void FModelRevision::PruneForUpdateCall(TSharedPtr<FJsonObject> CompiledDataJsonObject)
{
	CompiledDataJsonObject->RemoveField("revisionIndex");
	CompiledDataJsonObject->RemoveField("revisionCreationTime");
	CompiledDataJsonObject->RemoveField("revisionVersions");
}

void FModelMetadata::PruneForUpdateCall(TSharedPtr<FJsonObject> CompiledDataJsonObject)
{
	//No prune needed.
}

void FModelInfo::PruneForUpdateCall(TSharedPtr<FJsonObject> CompiledDataJsonObject)
{
	CompiledDataJsonObject->RemoveField("modelId");
	CompiledDataJsonObject->RemoveField("modelRevisions");
	CompiledDataJsonObject->RemoveField("modelSharedWithUserIds");
}

void FShareInfo::PruneForUpdateCall(TSharedPtr<FJsonObject> CompiledDataJsonObject)
{
	if (shareWithAll)
	{
		CompiledDataJsonObject->RemoveField("userIds");
	}
	else
	{
		CompiledDataJsonObject->RemoveField("shareWithAll");
	}

}
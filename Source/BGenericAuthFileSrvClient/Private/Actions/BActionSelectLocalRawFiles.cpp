/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "Actions/BActionSelectLocalRawFiles.h"
#include "Managers/BManagerStore.h"
#include "Managers/BActionManager.h"
#include "Engine/LatentActionManager.h"
#include "JsonUtilities.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Data/BProcessData.h"
#include "GenericPlatform/GenericPlatformMisc.h"

void UBKActionSelectLocalRawFiles::Execute()
{
	Super::Execute();

	TArray<FString> FilePaths;
	FString TopAssemblyFormatFileDialogFriendIfAny;
	FString TopAssemblyFileNameIfAny;
	TArray<FString> PotentialCADSoftwareNames;
	FString SelectedDirectoryPath;

	FString FileEntryName;

	if (!Execute_PickFiles(FilePaths, TopAssemblyFormatFileDialogFriendIfAny, PotentialCADSoftwareNames, FileEntryName, SelectedDirectoryPath)) return;
	if (Input.DialogType != EBDialogType::DirectoryForDownload)
	{
		if (TopAssemblyFormatFileDialogFriendIfAny.Len() > 0)
		{
			if (!Execute_PickTopAssembly(TopAssemblyFormatFileDialogFriendIfAny, TopAssemblyFileNameIfAny)) return;
		}

		if (TopAssemblyFileNameIfAny.Len() > 0)
		{
			//Override with top assembly name
			FileEntryName = TopAssemblyFileNameIfAny;
		}
	}

	if (IsActionCanceled()) return;

	OnSelectLocalRawFilesSucceed.Broadcast(FilePaths, TopAssemblyFileNameIfAny, PotentialCADSoftwareNames, FileEntryName, SelectedDirectoryPath);
}

bool UBKActionSelectLocalRawFiles::Execute_PickFiles(TArray<FString>& OutFilePaths, FString& OutTopAssemblyFormatFileDialogFriendIfAny, TArray<FString>& OutPotentialCADSoftwares, FString& BestDisplayName, FString& OutSelectedDirectoryPath)
{
	if (IsActionCanceled()) return false;

	void* ParentWindowHandle = FSlateApplication::Get().GetActiveTopLevelWindow()->GetNativeWindow()->GetOSWindowHandle();

	bool bResult;
	if (Input.DialogType == EBDialogType::FilesForUpload)
	{
		bResult = FDesktopPlatformModule::Get()->OpenFileDialog(
			ParentWindowHandle,
			TEXT("Select CAD files"),
			FPaths::GetProjectFilePath(),
			FString(""),
			BSupportedFileFormats::Get().GetFormatsFileDialogFriendly(),
			1,
			OutFilePaths);

		if (bResult)
		{
			if (OutFilePaths.Num() > 0)
			{
				OutSelectedDirectoryPath = FPaths::GetPath(OutFilePaths[0]);
				OutSelectedDirectoryPath.RemoveFromEnd("/");
				OutSelectedDirectoryPath.RemoveFromEnd("\\");
				int64 BiggestFileSize = -1;
				for (auto& Path : OutFilePaths)
				{
					int64 Size = IFileManager::Get().FileSize(*Path);
					if (BiggestFileSize < Size)
					{
						BiggestFileSize = Size;
						BestDisplayName = FPaths::GetBaseFilename(Path);
					}
				}
			}
		}
	}
	else if(Input.DialogType == EBDialogType::DirectoryForUpload)
	{
		TArray<FString> OutFileNames;

		bResult = FDesktopPlatformModule::Get()->OpenDirectoryDialog(
			ParentWindowHandle,
			TEXT("Select a folder containing CAD files"),
			FPaths::GetProjectFilePath(),
			OutSelectedDirectoryPath);

		if (bResult)
		{
			IFileManager::Get().FindFiles(OutFileNames, *OutSelectedDirectoryPath);

			OutSelectedDirectoryPath.RemoveFromEnd("/");
			OutSelectedDirectoryPath.RemoveFromEnd("\\");
			BestDisplayName = FPaths::GetBaseFilename(OutSelectedDirectoryPath + ".b", true);
			FString FolderNameWithSlash = OutSelectedDirectoryPath / "";

			for (int32 i = 0; i < OutFileNames.Num(); i++)
			{
				OutFilePaths.Add(FolderNameWithSlash + OutFileNames[i]);
			}
		}
	}
	else
	{
		bResult = FDesktopPlatformModule::Get()->OpenDirectoryDialog(
			ParentWindowHandle,
			TEXT("Select a folder containing CAD files"),
			FPaths::GetProjectFilePath(),
			OutSelectedDirectoryPath);

		if (bResult)
		{
			OutSelectedDirectoryPath.RemoveFromEnd("/");
			OutSelectedDirectoryPath.RemoveFromEnd("\\");
			BestDisplayName = FPaths::GetBaseFilename(OutSelectedDirectoryPath + ".b", true);
			return true;
		}
	}

	if (!bResult)
	{
		OnSelectLocalRawFilesFailed.Broadcast(TEXT("Dialog closed."));
		return false;
	}

	if (OutFilePaths.Num() == 0)
	{
		OnSelectLocalRawFilesFailed.Broadcast(
			Input.DialogType == EBDialogType::FilesForUpload ?
			TEXT("No file selected.") :
			TEXT("Selected folder (if any) does not contain any file.")
		);
		return false;
	}

	BSupportedFileFormats::Get().FilterOutUnsupported(OutFilePaths);

	OutPotentialCADSoftwares = BSupportedFileFormats::Get().GetGroupFromFiles(OutFilePaths);
	BSupportedFileFormats::Get().FilterOutFilesExceptGivenGroups(OutFilePaths, OutPotentialCADSoftwares);

	if (OutFilePaths.Num() == 0)
	{
		OnSelectLocalRawFilesFailed.Broadcast(
			Input.DialogType == EBDialogType::FilesForUpload ?
			TEXT("No supported file type selected.") :
			TEXT("Selected folder does not contain any supported file format.")
		);
		return false;
	}

	for (auto& Group : OutPotentialCADSoftwares)
	{
		FString Tmp;
		BSupportedFileFormats::Get().TryGetTopAssemblyFileDialogFriendlyForGroup(Group, Tmp);
		OutTopAssemblyFormatFileDialogFriendIfAny += Tmp;
		OutTopAssemblyFormatFileDialogFriendIfAny += "|";
	}
	OutTopAssemblyFormatFileDialogFriendIfAny.RemoveFromEnd("|");

	return true;
}

bool UBKActionSelectLocalRawFiles::Execute_PickTopAssembly(const FString& TopAssemblyFormatFileDialogFriendIfAny, FString& TopAssemblyFileNameIfAny)
{
	if (IsActionCanceled()) return false;

	void* ParentWindowHandle = FSlateApplication::Get().GetActiveTopLevelWindow()->GetNativeWindow()->GetOSWindowHandle();

	TArray<FString> SelectedFilePathAsArray;

	bool bResult = FDesktopPlatformModule::Get()->OpenFileDialog(
		ParentWindowHandle,
		TEXT("Select the top assembly file"),
		FPaths::GetProjectFilePath(),
		FString(""),
		TopAssemblyFormatFileDialogFriendIfAny,
		0,
		SelectedFilePathAsArray);

	if (bResult && !TopAssemblyFormatFileDialogFriendIfAny.IsEmpty())
	{
		TopAssemblyFileNameIfAny = FPaths::GetCleanFilename(SelectedFilePathAsArray[0]);
	}

	return true;
}

void UBKActionSelectLocalRawFiles::Perform_Select_Local_Raw_Files_Action_BP(
	const FBKActionSelectLocalRawFilesInput& Input,
	BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL& Exec,
	FBKActionSelectLocalRawFilesOutput& Output,
	FLatentActionInfo LatentInfo)
{
	UBKActionSelectLocalRawFiles* Action = NewObject<UBKActionSelectLocalRawFiles>();
	Action->Input = Input;
	Action->Output.RelativeInput = Action->Input;
	Action->Output.InputPtr = &Action->Input;
	Action->SetInputOutputPointers((FBKInputBase*)&Input, (FBKOutputBase*)&Output);

	Action->CalledFromBP(Exec, LatentInfo, &Action->Output, &Output);

	Action->OnSelectLocalRawFilesFailed.AddWeakLambda(Action, [Action]
		(const FString& Message)
		{
			if (Action->Output.Exec)
				*Action->Output.Exec = BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL::Failed;

			Action->Output.ErrorMessage = Message;

			if (Action->Output.Condition.IsValid())
				*Action->Output.Condition.Get() = true;

			Action->ActionCompleted();
		});
	Action->OnSelectLocalRawFilesSucceed.AddWeakLambda(Action, [Action]
		(const TArray<FString>& SelectedFilePaths, const FString& TopAssemblyFileNameIfAny, const TArray<FString>& PotentialCADSoftwareNames, const FString& FileEntryName, const FString& SelectedDirectoryPath)
		{
			if (Action->Output.Exec)
				*Action->Output.Exec = BK_ACTION_W_PROGRESS_BP_OUT_EXEC_INTERNAL::Succeed;

			Action->Output.SelectedFilePaths = SelectedFilePaths;
			Action->Output.TopAssemblyFileNameIfAny = TopAssemblyFileNameIfAny;
			Action->Output.PotentialCADSoftwareNames = PotentialCADSoftwareNames;
			Action->Output.FileEntryName = FileEntryName;
			Action->Output.SelectedDirectoryPath = SelectedDirectoryPath;

			if (Action->Output.Condition.IsValid())
				*Action->Output.Condition.Get() = true;

			Action->ActionCompleted();
		});

	UBKManagerStore::GetActionManager()->QueueAction(Action);
}

void UBKActionSelectLocalRawFiles::Perform_Select_Local_Raw_Files_Action_Cpp(
	const FBKActionSelectLocalRawFilesInput& Input,
	const TFunction<void(const FString&)>& FailedCallback,
	const TFunction<void(const TArray<FString>&, const FString&, const TArray<FString>&, const FString&, const FString&)>& SucceedCallback)
{
	UBKActionSelectLocalRawFiles* Action = NewObject<UBKActionSelectLocalRawFiles>();
	Action->Input = Input;
	Action->SetInputOutputPointers((FBKInputBase*)&Input, nullptr);

	Action->OnSelectLocalRawFilesFailed.AddWeakLambda(Action, [Action, FailedCallback]
		(const FString& Message)
		{
			FailedCallback(Message);
			Action->ActionCompleted();
		});
	Action->OnSelectLocalRawFilesSucceed.AddWeakLambda(Action, [Action, SucceedCallback]
		(const TArray<FString>& SelectedFilePaths, const FString& TopAssemblyFileNameIfAny, const TArray<FString>& PotentialCADSoftwareNames, const FString& FileEntryName, const FString& SelectedDirectoryPath)
		{
			SucceedCallback(SelectedFilePaths, TopAssemblyFileNameIfAny, PotentialCADSoftwareNames, FileEntryName, SelectedDirectoryPath);
			Action->ActionCompleted();
		});

	UBKManagerStore::GetActionManager()->QueueAction(Action);
}
/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "Data/BProcessData.h"

BSupportedFileFormats::BSupportedFileFormats()
{
	CreateFormatToDescriptionMap();
	CompileDescriptionToFormatsMap();
	CompileGroups();
	CompileForDelimited();
	CompileFileDialogFriendly();
	CompileTopAssembliesFileDialogFriendly();
}

const FString& BSupportedFileFormats::GetFormatsFileDialogFriendly()
{
	return FormatsFileDialogFriendly;
}

void BSupportedFileFormats::CreateFormatToDescriptionMap()
{
	F2D.Add(".rvm", FBFormat("Aveva"));
	F2D.Add(".att", FBFormat("Aveva"));

	F2D.Add(".sldasm", FBFormat("Solidworks", ".sldasm")); 
	F2D.Add(".sldprt", FBFormat("Solidworks", ".sldasm"));

	F2D.Add(".pxz", FBFormat("PiXYZ"));

	F2D.Add(".3ds", FBFormat("Autodesk 3ds Max"));

	F2D.Add(".sat", FBFormat("ACIS")); 
	F2D.Add(".sab", FBFormat("ACIS"));

	F2D.Add(".dwg", FBFormat("Autodesk AutoCAD 3D")); 
	F2D.Add(".dxf", FBFormat("Autodesk AutoCAD 3D"));

	F2D.Add(".fbx", FBFormat("Autodesk FBX"));

	F2D.Add(".ipt", FBFormat("Autodesk Inventor", ".iam")); 
	F2D.Add(".iam", FBFormat("Autodesk Inventor", ".iam"));

	F2D.Add(".rvt", FBFormat("Autodesk Revit")); 
	F2D.Add(".rfa", FBFormat("Autodesk Revit"));

	F2D.Add(".model", FBFormat("CATIA", ".catproduct"));
	F2D.Add(".session", FBFormat("CATIA", ".catproduct"));
	F2D.Add(".catpart", FBFormat("CATIA", ".catproduct"));
	F2D.Add(".catproduct", FBFormat("CATIA", ".catproduct"));
	F2D.Add(".catshape", FBFormat("CATIA", ".catproduct"));
	F2D.Add(".cgr", FBFormat("CATIA", ".catproduct"));
	F2D.Add(".3dxml", FBFormat("CATIA", ".catproduct"));

	F2D.Add(".asm", FBFormat("PTC - Creo - Pro/Engineer", ".asm")); 
	F2D.Add(".neu", FBFormat("PTC - Creo - Pro/Engineer", ".asm"));
	F2D.Add(".prt", FBFormat("PTC - Creo - Pro/Engineer", ".asm"));
	F2D.Add(".xas", FBFormat("PTC - Creo - Pro/Engineer", ".asm"));
	F2D.Add(".xpr", FBFormat("PTC - Creo - Pro/Engineer", ".asm"));

	F2D.Add(".dae", FBFormat("COLLADA"));

	F2D.Add(".csb", FBFormat("CSB Deltagen"));

	F2D.Add(".gltf", FBFormat("glTF")); 
	F2D.Add(".glb", FBFormat("glTF"));

	F2D.Add(".ifc", FBFormat("IFC"));

	F2D.Add(".igs", FBFormat("IGES")); 
	F2D.Add(".iges", FBFormat("IGES"));

	F2D.Add(".jt", FBFormat("Siemens JT"));

	F2D.Add(".obj", FBFormat("OBJ"));

	F2D.Add(".x_b", FBFormat("Parasolid")); 
	F2D.Add(".x_t", FBFormat("Parasolid")); 
	F2D.Add(".p_t", FBFormat("Parasolid")); 
	F2D.Add(".p_b", FBFormat("Parasolid")); 
	F2D.Add(".xmt", FBFormat("Parasolid")); 
	F2D.Add(".xmt_txt", FBFormat("Parasolid")); 
	F2D.Add(".xmt_bin", FBFormat("Parasolid"));

	F2D.Add(".pdf", FBFormat("PDF"));

	F2D.Add(".plmxml", FBFormat("Siemens PLM"));

	F2D.Add(".e57", FBFormat("Point Cloud")); 
	F2D.Add(".pts", FBFormat("Point Cloud")); 
	F2D.Add(".ptx", FBFormat("Point Cloud"));

	F2D.Add(".prc", FBFormat("Adobe Acrobat 3D"));

	F2D.Add(".3dm", FBFormat("Rhino3D"));

	F2D.Add(".skp", FBFormat("SketchUp"));

	F2D.Add(".asm", FBFormat("Siemens Solid Edge", ".asm")); 
	F2D.Add(".par", FBFormat("Siemens Solid Edge", ".asm"));
	F2D.Add(".pwd", FBFormat("Siemens Solid Edge", ".asm"));
	F2D.Add(".psm", FBFormat("Siemens Solid Edge", ".asm"));

	F2D.Add(".stp", FBFormat("STEP")); 
	F2D.Add(".step", FBFormat("STEP")); 
	F2D.Add(".stpz", FBFormat("STEP")); 
	F2D.Add(".stepz", FBFormat("STEP"));

	F2D.Add(".stl", FBFormat("StereoLithography"));

	F2D.Add(".u3d", FBFormat("U3D"));

	F2D.Add(".prt", FBFormat("Siemens Unigraphics-NX"));

	F2D.Add(".vda", FBFormat("VDA-FS"));

	F2D.Add(".wrl", FBFormat("VRML")); 
	F2D.Add(".wrml", FBFormat("VRML"));
}

void BSupportedFileFormats::CompileDescriptionToFormatsMap()
{
	for (auto& Pair : F2D)
	{
		if (!D2F.Contains(Pair.Value.Description))
		{
			D2F.Add(Pair.Value.Description, TArray<FString>());
		}
		D2F[Pair.Value.Description].AddUnique(Pair.Key);
	}
}

void BSupportedFileFormats::CompileGroups()
{
	for (auto& Pair : F2D)
	{
		if (!F2Groups.Contains(Pair.Key))
		{
			F2Groups.Add(Pair.Key, TArray<FString>());
		}
		F2Groups[Pair.Key].AddUnique(Pair.Value.Description);

		if (!GroupToFormats.Contains(Pair.Value.Description))
		{
			GroupToFormats.Add(Pair.Value.Description, TArray<FString>());
		}
		GroupToFormats[Pair.Value.Description].Add(Pair.Key);
	}
}

void BSupportedFileFormats::CompileForDelimited()
{
	for (auto& Pair : F2D)
	{
		FormatsDelimited += "*";
		FormatsDelimited += Pair.Key;
		FormatsDelimited += ";";
	}
	FormatsDelimited.RemoveFromEnd(";");
}

void BSupportedFileFormats::CompileFileDialogFriendly()
{
	for (auto& Pair : D2F)
	{
		FormatsFileDialogFriendly += Pair.Key;
		FormatsFileDialogFriendly += "|";

		for (auto& Format : Pair.Value)
		{
			FormatsFileDialogFriendly += "*";
			FormatsFileDialogFriendly += Format;
			FormatsFileDialogFriendly += ";";
		}
		FormatsFileDialogFriendly.RemoveFromEnd(";");
		FormatsFileDialogFriendly += "|";
	}
	FormatsFileDialogFriendly.RemoveFromEnd("|");

	FormatsFileDialogFriendly += "|All supported formats|";
	FormatsFileDialogFriendly += FormatsDelimited;
}

void BSupportedFileFormats::CompileTopAssembliesFileDialogFriendly()
{
	for (auto& Pair : F2D)
	{
		if (Pair.Value.StructureDefinition == EBFormatStructure::TopAssemblyDriven)
		{
			if (!FormatToTopAssemblyFileDialogFriendly.Contains(Pair.Key))
			{
				FormatToTopAssemblyFileDialogFriendly.Add(Pair.Key,
					Pair.Value.Description +
					" Top Assembly File|*" +
					Pair.Value.TopAssemblyFormat);
			}
			else
			{
				FormatToTopAssemblyFileDialogFriendly[Pair.Key] = (FormatToTopAssemblyFileDialogFriendly[Pair.Key] +
					"|" + 
					Pair.Value.Description +
					" Top Assembly File|*" +
					Pair.Value.TopAssemblyFormat);
			}

			if (!GroupToTopAssemblyFileDialogFriendly.Contains(Pair.Value.Description))
			{
				GroupToTopAssemblyFileDialogFriendly.Add(Pair.Value.Description,
					Pair.Value.Description +
					" Top Assembly File|*" +
					Pair.Value.TopAssemblyFormat);
			}
		}
	}
}

void BSupportedFileFormats::FilterOutUnsupported(TArray<FString>& FilePaths)
{
	for (int32 i = FilePaths.Num() - 1; i >= 0; i--)
	{
		if (!F2D.Contains(FPaths::GetExtension(FilePaths[i], true).ToLower()))
		{
			FilePaths.RemoveAt(i);
		}
	}
}

void BSupportedFileFormats::FilterOutFilesExceptGivenGroups(TArray<FString>& FilePaths, const TArray<FString>& Groups)
{
	for (int32 i = FilePaths.Num() - 1; i >= 0; i--)
	{
		FString Extension = FPaths::GetExtension(FilePaths[i], true);

		if (!F2Groups.Contains(Extension)) continue;

		bool bExists = false;
		for (auto& Group : Groups)
		{
			for (auto& GroupCompareWith : F2Groups[Extension])
			{
				if (Group == GroupCompareWith)
				{
					bExists = true;
					break;
				}
			}
			if (bExists) break;
		}

		if (!bExists)
		{
			FilePaths.RemoveAt(i);
		}
	}
}

TArray<FString> BSupportedFileFormats::GetGroupFromFiles(const TArray<FString>& FilePaths)
{
	TMap<FString, int32> Group2Occurence;
	int32 i = 0;
	for (auto& FilePath : FilePaths)
	{
		FString Extension = FPaths::GetExtension(FilePath, true);

		if (!F2Groups.Contains(Extension)) continue;

		for (auto& Group : F2Groups[Extension])
		{
			if (!Group2Occurence.Contains(Group))
			{
				Group2Occurence.Add(Group, 1);
			}
			else
			{
				Group2Occurence[Group] = (Group2Occurence[Group] + 1);
			}
		}
		i++;
	}

	TArray<FString> Result;
	if (Group2Occurence.Num() > 0)
	{
		int32 HighestOccurence = -1;
		FString HighestOccured;
		for (auto& Pair : Group2Occurence)
		{
			if (Pair.Value > HighestOccurence)
			{
				HighestOccurence = Pair.Value;
				HighestOccured = Pair.Key;

				Result.Empty();
			}
			else if (Pair.Value == HighestOccurence)
			{
				Result.Add(Pair.Key);
			}
		}
		Result.Add(HighestOccured);
	}
	return Result;
}

bool BSupportedFileFormats::TryGetTopAssemblyFileDialogFriendlyForFileFormat(const FString& FileFormat, FString& TopAssemblyFileDialogFriendly)
{
	if (FormatToTopAssemblyFileDialogFriendly.Contains(FileFormat))
	{
		TopAssemblyFileDialogFriendly = FormatToTopAssemblyFileDialogFriendly[FileFormat];
		return true;
	}
	return false;
}

bool BSupportedFileFormats::TryGetTopAssemblyFileDialogFriendlyForGroup(const FString& Group, FString& TopAssemblyFileDialogFriendly)
{
	if (GroupToTopAssemblyFileDialogFriendly.Contains(Group))
	{
		TopAssemblyFileDialogFriendly = GroupToTopAssemblyFileDialogFriendly[Group];
		return true;
	}
	return false;
}
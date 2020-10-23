/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

using UnrealBuildTool;

public class BGenericAuthFileSrvClient : ModuleRules
{
	public BGenericAuthFileSrvClient(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
        PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"zlib",
				"BZipLib",
				"BHttpServerLib",
				"BUtilities",
                "Json",
				"JsonUtilities",
				"HTTP",
                "DesktopPlatform",
				"UnrealEd"
			});
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",
				"InputCore",
				"UMG",
                "CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                "UMGEditor",
                "LevelEditor",
                "UnrealEd",
                "Blutility"
            });
	}
}
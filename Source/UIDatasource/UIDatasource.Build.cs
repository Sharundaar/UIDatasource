// Copyright Sharundaar. All Rights Reserved.

using UnrealBuildTool;

public class UIDatasource : ModuleRules
{
	public UIDatasource(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"GameplayTags",
				"StructUtils",
			}
		);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"TraceLog",
				"UMG",	
			}
		);
	}
}

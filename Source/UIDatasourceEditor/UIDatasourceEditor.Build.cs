// Copyright Sharundaar. All Rights Reserved.

using UnrealBuildTool;

public class UIDatasourceEditor : ModuleRules
{
    public UIDatasourceEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core", "UMGEditor", "UIDatasource",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "UMG",
                "Kismet",
                "InputCore",
                "BlueprintGraph",
                "UnrealEd",
                "GameplayTags",
                "KismetCompiler",
                "GraphEditor",
                "WorkspaceMenuStructure",
                "LiveCoding",
                "PropertyEditor",
				"StructUtils",
            }
        );
    }
}
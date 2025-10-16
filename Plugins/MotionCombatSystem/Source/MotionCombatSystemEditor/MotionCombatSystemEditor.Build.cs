/*
 * ========================================================================
 * Copyright © 2025 God's Studio
 * All Rights Reserved.
 *
 * Project: Motion Combat System
 * Author: Christopher D. Parker
 * Date: 10-15-2025
 * =============================================================================
 * MotionCombatSystemEditor.Build.cs
 * Build script for the editor-only module that adds MCS custom menu actions.
 */

using UnrealBuildTool;

public class MotionCombatSystemEditor : ModuleRules
{
    public MotionCombatSystemEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "Slate",
            "SlateCore",
            "UnrealEd",
            "AssetTools",
            "ContentBrowser",
            "ToolMenus",
            "DataTableEditor",
            "MotionCombatSystem"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "EditorFramework",
            "Kismet"
        });
    }
}

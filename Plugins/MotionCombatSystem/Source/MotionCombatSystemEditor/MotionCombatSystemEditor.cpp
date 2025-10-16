#include "MotionCombatSystemEditor.h"
#include "Modules/ModuleManager.h"
#include "AssetToolsModule.h"
#include "ClassIconFinder.h"
#include "Style/MotionCombatSystemEditorStyle.h"


IMPLEMENT_MODULE(FMotionCombatSystemEditorModule, MotionCombatSystemEditor)


EAssetTypeCategories::Type MotionCombatSystemCategory;


/*
 * Register a new top-level asset category for the plugin
 * This is optional, but helps keep things organized
*/
void FMotionCombatSystemEditorModule::StartupModule()
{
    FMotionCombatSystemEditorStyle::Initialize();

    IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

    // Register a new top-level category
    MotionCombatSystemCategory = AssetTools.RegisterAdvancedAssetCategory(
        FName(TEXT("MotionCombatSystem")),      // Internal name
        FText::FromString("Motion Combat System") // Displayed name in Add New
    );
}

/*
 * Unregister all asset types created in StartupModule
*/
void FMotionCombatSystemEditorModule::ShutdownModule()
{
    FMotionCombatSystemEditorStyle::Shutdown();
}

#undef LOCTEXT_NAMESPACE
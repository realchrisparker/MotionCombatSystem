/*
 * ========================================================================
 * Copyright © 2025 God's Studio
 * All Rights Reserved.
 *
 * Free for all to use, copy, and distribute. I hope you learn from this as I learned creating it.
 * =============================================================================
 *
 * Project: Motion Combat System
 * This is a combat system inspired by Unreal Engine’s Motion Matching plugin.
 * Author: Christopher D. Parker
 * Date: 10-15-2025
 * =============================================================================
 * AttackDataTableFactory.h
 * Factory that creates a UDataTable preconfigured with FMCS_AttackEntry as the row struct.
 */

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "AssetToolsModule.h"
#include "AssetTypeCategories.h"
#include <Structs/MCS_AttackEntry.h>
#include "AttackDataTableFactory.generated.h"

extern EAssetTypeCategories::Type MotionCombatSystemCategory;

UCLASS()
class MOTIONCOMBATSYSTEMEDITOR_API UAttackDataTableFactory : public UFactory
{
    GENERATED_BODY()

public:
    // Constructor
    UAttackDataTableFactory()
    {
        bCreateNew = true;
        bEditAfterNew = true;
        SupportedClass = UDataTable::StaticClass();
    }

    /**
     * Factory method to create a new instance of the supported class.
     * @param Class The class of the object to create.
     * @param InParent The parent object for the new object.
     * @param Name The name of the new object.
     * @param Flags The flags to apply to the new object.
     * @param Context The context in which the object is being created.
     * @param Warn The feedback context for warnings.
     * @return A new instance of the supported class.
     */
    virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override
    {
        // Create the DataTable asset in the exact path the user selected.
        // InParent is the package corresponding to the selected folder, so we
        // simply spawn the UDataTable there and set its RowStruct.

        // 1) Create the DataTable object
        UDataTable* NewTable = NewObject<UDataTable>(InParent, Class ? Class : UDataTable::StaticClass(), Name, Flags);
        if (!NewTable)
        {
            return nullptr;
        }

        // 2) Assign our row struct (CRITICAL)
        NewTable->RowStruct = FMCS_AttackEntry::StaticStruct();

        // 3) Optional niceties: mark dirty so it prompts to save
        NewTable->Modify();

        // Done — the editor will open it automatically because bEditAfterNew = true (and ShouldOpenInEditor)
        return NewTable;
    }

    /*
     * Get the menu categories for this factory.
     * @return The menu categories for this factory.
     */
    virtual uint32 GetMenuCategories() const override
    {
        return MotionCombatSystemCategory; //EAssetTypeCategories::Misc;
    }

    /**
     * Determine whether this factory should be shown in the "New" menu.
     * @return True if the factory should be shown, false otherwise.
     */
    virtual bool ShouldShowInNewMenu() const override
    {
        return true;
    }

    /*
    * Get the display name for this factory.
    * @return The display name for this factory.
    */
    virtual FText GetDisplayName() const override
    {
        return NSLOCTEXT("AttackDataTableFactory", "DisplayName", "Attack Data Table");
    }

    /*
    * Get the tooltip for this factory.
    * @return The tooltip for this factory.
    */
    virtual FText GetToolTip() const override
    {
        return NSLOCTEXT("AttackDataTableFactory", "ToolTip", "Create a new Motion Combat System Attack Data Table.");
    }
};

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
 * AttackChooserFactory.h
 * Factory that creates a UMCS_AttackChooser object.
 */

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "AssetToolsModule.h"
#include "AssetTypeCategories.h"
#include "Kismet2/KismetEditorUtilities.h"
#include <Choosers/MCS_AttackChooser.h>
#include "AttackChooserFactory.generated.h"

extern EAssetTypeCategories::Type MotionCombatSystemCategory;

UCLASS()
class MOTIONCOMBATSYSTEMEDITOR_API UAttackChooserFactory : public UFactory
{
    GENERATED_BODY()

public:
    // Constructor
    UAttackChooserFactory()
    {
        bCreateNew = true;
        bEditAfterNew = true;
        SupportedClass = UBlueprint::StaticClass(); //UMCS_AttackChooser
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
        // Create a Blueprint with APGAS_EnemyCharacter as the parent
        UClass* ParentClass = UMCS_AttackChooser::StaticClass();

        return FKismetEditorUtilities::CreateBlueprint(
            ParentClass,
            InParent,
            Name,
            BPTYPE_Normal,              // Normal Blueprint
            UBlueprint::StaticClass(),  // Blueprint asset type
            UBlueprintGeneratedClass::StaticClass(),
            FName("AttackChooserFactory")
        );

        // return NewObject<UMCS_AttackChooser>(InParent, Class ? Class : UMCS_AttackChooser::StaticClass(), Name, Flags);
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
        return NSLOCTEXT("AttackChooserFactory", "DisplayName", "Attack Chooser");
    }

    /*
    * Get the tooltip for this factory.
    * @return The tooltip for this factory.
    */
    virtual FText GetToolTip() const override
    {
        return NSLOCTEXT("AttackChooserFactory", "ToolTip", "Create a new Motion Combat System Attack Chooser.");
    }
};

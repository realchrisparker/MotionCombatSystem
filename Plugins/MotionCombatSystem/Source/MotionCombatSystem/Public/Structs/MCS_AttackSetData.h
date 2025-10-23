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
 * Date: 10-14-2025
 * =============================================================================
 * MCS_AttackSetData.h
 * Represents an attack set in the MCS Combat System.
 */

#pragma once

#include "CoreMinimal.h"
#include <Choosers/MCS_AttackChooser.h>
#include "MCS_AttackSetData.generated.h"

USTRUCT(BlueprintType, Blueprintable, meta = (DisplayName = "Motion Combat System Attack Set", Description = "Represents an attack set in the MCS Combat System"))
struct MOTIONCOMBATSYSTEM_API FMCS_AttackSetData
{
    GENERATED_BODY()

public:

    /** DataTable containing the attack entries for this set */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Combat", meta = (DisplayName = "Attack Data Table", Description = "DataTable containing the attack entries for this set"))
    TObjectPtr<UDataTable> AttackDataTable = nullptr;

    /** Attack chooser used for this set */
    UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, Category = "MCS|Combat", meta = (DisplayName = "Attack Chooser", Description = "Attack chooser used for this set"))
    TObjectPtr<UMCS_AttackChooser> AttackChooser = nullptr;
};
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
 * FMCS_AttackEntry.h
 * Declares the FMCS_AttackEntry struct used to describe an attack entry stored in
 * DataTables and referenced by the Motion Combat System (chooser & core).
 */

#pragma once

#include "CoreMinimal.h"
#include <Enums/EMCS_ComparisonMethod.h>
#include "MCS_AttackCondition.generated.h"

 /**
  * FMCS_AttackCondition
  * Struct defining a condition that must be met for an attack to be considered valid.
  */
USTRUCT(BlueprintType, meta = (DisplayName = "Motion Combat System Attack Condition"))
struct MOTIONCOMBATSYSTEM_API FMCS_AttackCondition
{
    GENERATED_BODY()

    /** Name of the contextual attribute (e.g., "Speed", "Altitude", "Stamina"). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Condition", meta = (DisplayName = "Attribute Name", ToolTip = "Name of the contextual attribute (e.g., 'Speed', 'Altitude', 'Stamina')."))
    FName AttributeName;

    /** Comparison operator used to test the value. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Condition", meta = (DisplayName = "Comparison Method", ToolTip = "Method used to compare the attribute value."))
    EMCS_ComparisonMethod Comparison = EMCS_ComparisonMethod::Greater;

    /** Threshold value for comparison. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Condition", meta = (DisplayName = "Threshold", ToolTip = "Threshold value for comparison."))
    float Threshold = 0.f;

    /** Weight or bonus added to the total score when this condition passes. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Condition", meta = (DisplayName = "Weight", ToolTip = "Weight or bonus added to the total score when this condition passes."))
    float Weight = 10.f;

    /** If true, failing this condition disqualifies the attack. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Condition", meta = (DisplayName="Must Pass",ToolTip = "If true, failing this condition disqualifies the attack. If false, it only affects the score."))
    bool bMustPass = false; // if true, failing disqualifies the attack, if false, just affects score.
};
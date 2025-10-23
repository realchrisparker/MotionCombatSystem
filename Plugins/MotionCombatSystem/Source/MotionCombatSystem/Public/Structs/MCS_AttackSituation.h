/*
 * ========================================================================
 * Copyright © 2025 God's Studio
 * All Rights Reserved.
 *
 * Free for all to use, copy, and distribute. I hope you learn from this as I learned creating it.
 * =============================================================================
 *
 * Project: Paragon GAS Sample
 * Author: Christopher D. Parker
 * Date: 9-6-2025
 * =============================================================================
 * MCS_AttackSituation.h
 *
 * Defines attack situation enums and struct for data-driven attack situations.
 */

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimMontage.h"
#include "MCS_AttackSituation.generated.h"

 /*
 * Struct defining various attack situations for motion matching.
*/
USTRUCT(BlueprintType, meta = (DisplayName = "Motion Combat System Attack Situation"))
struct FMCS_AttackSituation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario", meta = (ToolTip = "Is the character grounded?"))
    bool bIsGrounded = true; // Default to grounded

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario", meta = (ToolTip = "Is the character in the air?"))
    bool bIsInAir = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario", meta = (ToolTip = "Is the character running?"))
    bool bIsRunning = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario", meta = (ToolTip = "Is the character crouching?"))
    bool bIsCrouching = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario", meta = (ToolTip = "Is the character countering?"))
    bool bIsCountering = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario", meta = (ToolTip = "Is the character parrying?"))
    bool bIsParrying = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario", meta = (ToolTip = "Is the character riposting?"))
    bool bIsRiposting = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario", meta = (ToolTip = "Is the character finishing enemy?"))
    bool bIsFinishing = false;
};
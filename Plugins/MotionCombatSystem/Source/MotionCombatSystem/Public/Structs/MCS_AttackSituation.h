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

public:
    
    /**
     * True if the character is currently on the ground.
     * Default is true since most attacks are grounded by default.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Situation", meta = (DisplayName = "Is Grounded"))
    bool bIsGrounded = true;

    /**
     * True if the character is currently airborne (jumping, falling, or in mid-air).
     * Typically used for aerial or jump attacks.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Situation", meta = (DisplayName = "Is In Air"))
    bool bIsInAir = false;

    /**
     * True if the character is moving faster than the running threshold.
     * Used for attacks that require forward momentum, such as sprint strikes or running kicks.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Situation", meta = (DisplayName = "Is Running"))
    bool bIsRunning = false;

    /**
     * True if the character is in a crouched or stealth stance.
     * Useful for low attacks, sweeps, or stealth-based moves.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Situation", meta = (DisplayName = "Is Crouching"))
    bool bIsCrouching = false;

    /**
     * True if the character is performing or has triggered a counterattack.
     * Commonly used for reactionary moves after blocking or evading an enemy attack.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Situation", meta = (DisplayName = "Is Countering"))
    bool bIsCountering = false;

    /**
     * True if the character is currently executing a parry.
     * Used for timing-based parry counterattacks that depend on precise player input.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Situation", meta = (DisplayName = "Is Parrying"))
    bool bIsParrying = false;

    /**
     * True if the character is performing a riposte (a follow-up after a successful parry or stagger event).
     * Typically triggers critical attacks or cinematic finishers.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Situation", meta = (DisplayName = "Is Riposting"))
    bool bIsRiposting = false;

    /**
     * True if the character is executing a finishing move.
     * Used for final-blow or low-health execution animations.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Situation", meta = (DisplayName = "Is Finishing"))
    bool bIsFinishing = false;

    /* =====================================================
     *  Quantitative Runtime Values
     * ===================================================== */

     /** Player’s current velocity magnitude. Used for conditions like “Speed > 600”. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Situation|Quantitative", meta = (DisplayName = "Current Speed"))
    float Speed = 0.f;

    /** Player’s current world Z position or jump height, if applicable. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Situation|Quantitative", meta = (DisplayName = "Altitude / Jump Height"))
    float Altitude = 0.f;

    /** Player’s current stamina level (if your system supports stamina scaling). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Situation|Quantitative", meta = (DisplayName = "Stamina"))
    float Stamina = 100.f;

    /** Player’s current health percentage, if you want to base conditions on HP. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Situation|Quantitative", meta = (DisplayName = "Health Percent"))
    float HealthPercent = 100.f;
};
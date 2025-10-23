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
 * EMCS_AttackSituation.h
 * Declares the EMCS_AttackSituation enum used to describe attack situations.
 */

#pragma once

#include "CoreMinimal.h"

/**
 * Enum to describe various attack situations for motion matching.
 */
UENUM(BlueprintType, meta = (DisplayName = "Motion Combat System Attack Situations"))
enum class EMCS_AttackSituations : uint8
{
    Grounded   UMETA(DisplayName = "Grounded"),
    Airborne   UMETA(DisplayName = "Airborne/Jumping"),
    Running    UMETA(DisplayName = "Running"),
    Crouching  UMETA(DisplayName = "Crouching"),
    Counter    UMETA(DisplayName = "Counter"),
    Parry      UMETA(DisplayName = "Parry"),
    Riposte    UMETA(DisplayName = "Riposte"),
    Finisher   UMETA(DisplayName = "Finisher"),
    Any        UMETA(DisplayName = "Any (Always Valid)")
};

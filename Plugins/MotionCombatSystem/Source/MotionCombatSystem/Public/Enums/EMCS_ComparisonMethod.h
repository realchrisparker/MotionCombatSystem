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
 * Date: 10-23-2025
 * =============================================================================
 * EMCS_ComparisonMethod.h
 * Declares the EMCS_ComparisonMethod enum used to describe comparison methods.
 */

#pragma once

#include "CoreMinimal.h"

/**
 * Simple data-driven condition check for advanced situational filtering.
 * Allows designers to define thresholds such as "Speed > 600".
 */
UENUM(BlueprintType, meta = (DisplayName = "Motion Combat System Comparison Method"))
enum class EMCS_ComparisonMethod : uint8
{
    Equal          UMETA(DisplayName = "Equal"),
    NotEqual       UMETA(DisplayName = "Not Equal"),
    Greater        UMETA(DisplayName = "Greater"),
    Less           UMETA(DisplayName = "Less"),
    GreaterOrEqual UMETA(DisplayName = "Greater Or Equal"),
    LessOrEqual    UMETA(DisplayName = "Less Or Equal")
};
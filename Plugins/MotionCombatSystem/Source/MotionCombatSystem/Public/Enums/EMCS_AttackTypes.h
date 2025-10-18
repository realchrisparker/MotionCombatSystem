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
 * EMCS_AttackTypes.h
 * Declares the EMCS_AttackType enum used to describe attack types.
 */

#pragma once

#include "CoreMinimal.h"

/**
 * Attack type enum - categorize attacks (used by chooser logic to filter/select).
 */
UENUM(BlueprintType, meta=(DisplayName="Motion Combat System Attack Type"))
enum class EMCS_AttackType : uint8
{
    Light      UMETA(DisplayName = "Light"),
    Heavy      UMETA(DisplayName = "Heavy"),
    Charge     UMETA(DisplayName = "Charge"),
    Thrown     UMETA(DisplayName = "Thrown"),
    Special    UMETA(DisplayName = "Special"),
    Unknown    UMETA(DisplayName = "Unknown")
};
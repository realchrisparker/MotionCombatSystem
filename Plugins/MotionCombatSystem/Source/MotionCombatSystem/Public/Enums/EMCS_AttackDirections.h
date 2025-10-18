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
 * EMCS_AttackDirections.h
 * Declares the EMCS_AttackDirection enum used to describe attack directions.
 */

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType, meta = (DisplayName = "Motion Combat System Attack Direction"))
enum class EMCS_AttackDirection : uint8
{
    Forward   UMETA(DisplayName = "Forward"),
    Left      UMETA(DisplayName = "Left"),
    Right     UMETA(DisplayName = "Right"),
    Backward  UMETA(DisplayName = "Backward"),
    Omni      UMETA(DisplayName = "Omni (Any Direction)")
};
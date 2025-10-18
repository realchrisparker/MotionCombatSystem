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
 * MCS_AttackHitbox.h
 * Represents an attack hitbox in the MCS Combat System.
 */

#pragma once

#include "CoreMinimal.h"
#include "MCS_AttackHitbox.generated.h"

USTRUCT(BlueprintType, Blueprintable, meta = (DisplayName = "Motion Combat System Attack Hitbox", Description = "Represents an attack hitbox in the MCS Combat System"))
struct MOTIONCOMBATSYSTEM_API FMCS_AttackHitbox
{
    GENERATED_BODY()

public:

    /** Start socket (e.g., "weapon_start"). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox", meta = (DisplayName = "Start Socket", Description = "Name of the socket marking the start of the hitbox sweep"))
    FName StartSocket = NAME_None;

    /** End socket (e.g., "weapon_end"). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox", meta = (DisplayName = "End Socket", Description = "Name of the socket marking the end of the hitbox sweep"))
    FName EndSocket = NAME_None;

    /** Sweep radius. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox", meta = (ClampMin = "0.0", DisplayName = "Radius", Description = "Radius of the hitbox sphere sweep"))
    float Radius = 10.f;

    /** Debug draw toggle for this attack. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox", meta = (DisplayName = "Debug Draw", Description = "Enable debug drawing for this hitbox"))
    bool bDebugDraw = true;
};
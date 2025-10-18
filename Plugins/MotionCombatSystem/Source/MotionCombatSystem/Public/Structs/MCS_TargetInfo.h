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
 * MCS_TargetInfo.h
 * Declares the FMCS_TargetInfo struct used to describe a target info entry stored in DataTables and referenced by the Motion Combat System (chooser & core).
 */

#pragma once

#include "CoreMinimal.h"
#include "MCS_TargetInfo.generated.h"

/**
 * Struct to hold targeting info for registered enemies.
 */
USTRUCT(BlueprintType, meta = (DisplayName = "Motion Combat System Target Info"))
struct FMCS_TargetInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
    TObjectPtr<AActor> TargetActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
    float DistanceFromPlayer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
    bool bIsValid = false;
};
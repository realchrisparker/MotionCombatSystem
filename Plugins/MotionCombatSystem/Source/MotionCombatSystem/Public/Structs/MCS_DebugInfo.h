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
 * MCS_DebugInfo.h
 * Defines runtime debug data used to display scoring overlays for Motion Combat System.
 */

#pragma once

#include "CoreMinimal.h"
#include "MCS_DebugInfo.generated.h"

USTRUCT(BlueprintType, meta = (DisplayName = "Motion Combat System Debug Attack Score"))
struct MOTIONCOMBATSYSTEM_API FMCS_DebugAttackScore
{
    GENERATED_BODY()

    /** Name of the attack as defined in DataTable. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MCS|Debug")
    FName AttackName;

    /** Total score after all factors. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MCS|Debug")
    float TotalScore = 0.f;

    /** Individual scoring components. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MCS|Debug")
    float BaseScore = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MCS|Debug")
    float TagScore = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MCS|Debug")
    float DistanceScore = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MCS|Debug")
    float DirectionScore = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MCS|Debug")
    float SituationScore = 0.f;

    /** Notes or condition summary (e.g., "Speed>600 passed"). */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MCS|Debug")
    FString Notes;

    /** True if this attack was ultimately chosen. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MCS|Debug")
    bool bWasChosen = false;
};

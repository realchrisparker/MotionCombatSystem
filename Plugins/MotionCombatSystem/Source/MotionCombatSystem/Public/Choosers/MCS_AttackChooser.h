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
 * MCS_AttackChooser.h
 * A Blueprint-friendly UObject that selects the best FMCS_AttackEntry from a list
 * based on modular scoring logic. Each part of the score calculation can now
 * be reused individually by designers inside Blueprint.
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include <Structs/MCS_AttackEntry.h>
#include <Structs/MCS_AttackSituation.h>
#include <Enums/EMCS_AttackDirections.h>
#include <Enums/EMCS_AttackSituations.h>
#include "GameplayTagContainer.h"
#include "MCS_AttackChooser.generated.h"

class AActor;

/**
 * UMCS_AttackChooser
 *
 * Selects an appropriate FMCS_AttackEntry based on scoring heuristics.
 * The scoring logic is now split into modular, BlueprintPure helper functions
 * (ComputeTagScore, ComputeDistanceScore, ComputeDirectionalScore, AggregateScore)
 * to improve designer usability.
 */
UCLASS(Blueprintable, BlueprintType, EditInlineNew, CollapseCategories, ClassGroup = (MotionCombatSystem), meta = (DisplayName = "Motion Combat System Attack Chooser"))
class MOTIONCOMBATSYSTEM_API UMCS_AttackChooser : public UObject
{
    GENERATED_BODY()

public:
    UMCS_AttackChooser();

    /* ==========================================================
     * Configurable Data
     * ========================================================== */

     /** Candidate attack entries (usually loaded from a DataTable). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|AttackChooser")
    TArray<FMCS_AttackEntry> AttackEntries;

    /** Maximum target distance considered valid. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|AttackChooser")
    float MaxTargetDistance;

    /** Maximum allowed facing angle (degrees). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|AttackChooser")
    float MaxTargetAngleDegrees;

    /** When multiple attacks tie, break ties randomly if true. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|AttackChooser")
    bool bRandomTieBreak;

    /** Optional tag filtering for attack selection. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|AttackChooser")
    FGameplayTag RequiredAttackTag;

    /** When true, unmatched tags are penalized instead of excluded. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|AttackChooser")
    bool bPreferTagInsteadOfFilter = false;

    /* ==========================================================
     * Public API
     * ========================================================== */

     /** Main function that chooses the best attack from AttackEntries. */
    UFUNCTION(BlueprintCallable, Category = "MCS|AttackChooser", meta = (DisplayName = "Choose Attack", ReturnDisplayName = "Was Attack Chosen"))
    bool ChooseAttack(
        AActor* Instigator,
        const TArray<AActor*>& Targets,
        EMCS_AttackDirection DesiredDirection,
        const FMCS_AttackSituation& CurrentSituation,
        FMCS_AttackEntry& OutAttack) const;

    /** Returns a copy of all loaded attack entries. */
    UFUNCTION(BlueprintCallable, Category = "MCS|AttackChooser", meta= (DisplayName = "Get Attack Entries", ReturnDisplayName = "Attack Entries"))
    TArray<FMCS_AttackEntry> GetAttackEntries() const { return AttackEntries; }

    /* ==========================================================
     * Scoring API (BlueprintPure helpers)
     * ========================================================== */

     /** Computes a score modifier based on tag filtering and preferences. */
    UFUNCTION(BlueprintPure, Category = "MCS|AttackChooser|Scoring", meta = (DisplayName = "Compute Tag Score", ReturnDisplayName = "Score"))
    float ComputeTagScore(const FMCS_AttackEntry& Entry) const;

    /** Computes a score modifier based on distance window. */
    UFUNCTION(BlueprintPure, Category = "MCS|AttackChooser|Scoring", meta = (DisplayName = "Compute Distance Score", ReturnDisplayName = "Score"))
    float ComputeDistanceScore(const FMCS_AttackEntry& Entry, AActor* Instigator, const TArray<AActor*>& Targets) const;

    /** Computes a score modifier based on desired attack direction. */
    UFUNCTION(BlueprintPure, Category = "MCS|AttackChooser|Scoring", meta = (DisplayName = "Compute Directional Score", ReturnDisplayName = "Score"))
    float ComputeDirectionalScore(const FMCS_AttackEntry& Entry, EMCS_AttackDirection DesiredDirection) const;

    /** Computes a score modifier based on the current combat situation. */
    UFUNCTION(BlueprintPure, Category = "MCS|Chooser|Scoring")
    float ComputeSituationScore(const FMCS_AttackEntry& Entry, const FMCS_AttackSituation& CurrentSituation) const;

    /** Combines individual score components into a final result. */
    UFUNCTION(BlueprintPure, Category = "MCS|AttackChooser|Scoring", meta = (DisplayName = "Aggregate Score", ReturnDisplayName = "Score"))
    float AggregateScore(float BaseScore, float TagScore, float DistanceScore, float DirectionScore, float SituationScore) const;

protected:
    /* ==========================================================
     * Core virtuals
     * ========================================================== */

     /** Core scoring entry point (BlueprintNativeEvent). */
    UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "MCS|AttackChooser", meta = (DisplayName = "Score Attack Entry", ReturnDisplayName = "Score"))
    float ScoreAttack(
        const FMCS_AttackEntry& Entry,
        AActor* Instigator,
        const TArray<AActor*>& Targets,
        EMCS_AttackDirection DesiredDirection,
        const FMCS_AttackSituation& CurrentSituation) const;
    virtual float ScoreAttack_Implementation(
        const FMCS_AttackEntry& Entry,
        AActor* Instigator,
        const TArray<AActor*>& Targets,
        EMCS_AttackDirection DesiredDirection,
        const FMCS_AttackSituation& CurrentSituation) const;

    /** Is entry allowed by basic filters (distance & angle). */
    bool IsEntryAllowedByBasicFilters(const FMCS_AttackEntry& Entry, AActor* Instigator, const TArray<AActor*>& Targets) const;
};

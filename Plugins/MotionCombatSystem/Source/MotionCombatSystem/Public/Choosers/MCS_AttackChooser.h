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
 * based on scoring logic. Designed to be overridable in Blueprints for designer
 * heuristics while providing a sensible C++ default implementation.
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include <Structs/MCS_AttackEntry.h>
#include "MCS_AttackChooser.generated.h"

class AActor;

/**
 * UMCS_AttackChooser
 *
 * A reusable chooser object that selects a suitable attack (FMCS_AttackEntry) from
 * an array of candidate entries. Designers can override ScoreAttack in Blueprints
 * to implement custom heuristics (distance, tags, target-facing, priority, etc).
 */
UCLASS(Blueprintable, BlueprintType, EditInlineNew, CollapseCategories, meta=(DisplayName="Motion Combat System Attack Chooser"))
class MOTIONCOMBATSYSTEM_API UMCS_AttackChooser : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // Constructor
    UMCS_AttackChooser();

    /*
     * Properties
    */

    /** Candidate attack entries to choose from. Fill in the Core Component or in a Blueprint data asset. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|AttackChooser", meta=(DisplayName="Attack Entries"))
    TArray<FMCS_AttackEntry> AttackEntries;

    /** Maximum distance to consider targets (default: 2500). If <= 0, distance filtering disabled. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|AttackChooser", meta=(DisplayName="Max Target Distance"))
    float MaxTargetDistance;

    /** Maximum allowed facing angle (degrees) between instigator forward and target for some attacks. If <= 0, angle filtering disabled. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|AttackChooser", meta=(DisplayName="Max Target Angle Degrees"))
    float MaxTargetAngleDegrees;

    /** When multiple attacks score the same, break ties randomly if true; otherwise pick the first. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|AttackChooser", meta=(DisplayName="Random Tie Break"))
    bool bRandomTieBreak;

    /** Optional: Only attacks that match this tag (or are children of it) will be considered valid. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|AttackChooser|Tags", meta = (DisplayName = "Required Attack Tag"))
    FGameplayTag RequiredAttackTag;

    /** Optional: When true, attacks that share the RequiredAttackTag get a small bonus instead of strict filtering. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|AttackChooser|Tags", meta = (DisplayName = "Prefer Matching Tag Instead Of Filter"))
    bool bPreferTagInsteadOfFilter = false;

    /*
     * Functions
    */

    /**
     * Choose an attack from AttackEntries based on scoring logic.
     * @param Instigator - the actor performing the attack (may be nullptr)
     * @param Targets - list of relevant targets (can be empty)
     * @param OutAttack - returned attack entry if the function returns true
     * @return true if a valid attack was chosen, false otherwise
     */
    UFUNCTION(BlueprintCallable, Category = "MCS|AttackChooser", meta=(DisplayName="Choose Attack"))
    bool ChooseAttack(AActor* Instigator, const TArray<AActor*>& Targets, FMCS_AttackEntry& OutAttack) const;

    /**
     * Get all attack entries (read-only copy)
     */
    UFUNCTION(BlueprintCallable, Category = "MCS|AttackChooser", meta=(DisplayName="Get Attack Entries"))
    TArray<FMCS_AttackEntry> GetAttackEntries() const { return AttackEntries; }

protected:
    /*
     * Functions
    */
    
    /**
     * BlueprintNativeEvent allows Blueprints to override the scoring heuristics.
     * Default implementation calls ScoreAttack_Implementation.
     *
     * @param Entry - candidate entry to score
     * @param Instigator - actor performing attack
     * @param Targets - array of targets to consider
     * @return a float score: higher = better. Return -TNumericLimits<float>::Max() to disqualify.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "MCS|AttackChooser", meta = (DisplayName = "Score Attack"))
    float ScoreAttack(const FMCS_AttackEntry& Entry, AActor* Instigator, const TArray<AActor*>& Targets) const;
    virtual float ScoreAttack_Implementation(const FMCS_AttackEntry& Entry, AActor* Instigator, const TArray<AActor*>& Targets) const;

    /**
     * Basic filtering based on MaxTargetDistance and MaxTargetAngleDegrees.
     * Can be used as a quick pre-check before more complex scoring in ScoreAttack.
     * @param Entry - candidate entry to check
     * @param Instigator - actor performing attack
     * @param Targets - array of targets to consider
     * @return true if the entry passes basic filters, false otherwise
     */
    bool IsEntryAllowedByBasicFilters(const FMCS_AttackEntry& Entry, AActor* Instigator, const TArray<AActor*>& Targets) const;

private:
    /** Internal flag to track if the DataTable was auto-loaded at least once. */
    bool bHasAutoLoaded = false;
};

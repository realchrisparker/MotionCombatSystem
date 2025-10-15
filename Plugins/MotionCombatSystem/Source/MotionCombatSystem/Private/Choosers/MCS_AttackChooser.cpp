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
 * MCS_AttackChooser.cpp
 * Implementation of UMCS_AttackChooser selection and default scoring.
 */

#include <Choosers/MCS_AttackChooser.h>
#include "GameFramework/Actor.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/DataTable.h"


 /*
  * Constructor
  */
UMCS_AttackChooser::UMCS_AttackChooser()
{
    MaxTargetDistance = 2500.0f;
    MaxTargetAngleDegrees = 180.0f; // default: allow all angles
    bRandomTieBreak = true;
}

/**
 * Choose an attack from AttackEntries based on scoring logic.
 * @param Instigator - the actor performing the attack (may be nullptr)
 * @param Targets - list of relevant targets (can be empty)
 * @param OutAttack - returned attack entry if the function returns true
 * @return true if a valid attack was chosen, false otherwise
 */
bool UMCS_AttackChooser::ChooseAttack(AActor* Instigator, const TArray<AActor*>& Targets, FMCS_AttackEntry& OutAttack) const
{
    if (AttackEntries.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("[MCS_AttackChooser] No attacks available to choose from."));
        return false;
    }

    // Track best score(s)
    float BestScore = -std::numeric_limits<float>::infinity();
    TArray<int32> BestIndices;
    BestIndices.Reserve(4);

    for (int32 Index = 0; Index < AttackEntries.Num(); ++Index)
    {
        const FMCS_AttackEntry& Entry = AttackEntries[Index];

        // Quick allowed check (distance/angle) - user can override more specifically in ScoreAttack
        if (!IsEntryAllowedByBasicFilters(Entry, Instigator, Targets))
        {
            continue;
        }

        // Score using BlueprintNativeEvent (can be overridden in Blueprint)
        const float Score = ScoreAttack(Entry, Instigator, Targets);

        if (!FMath::IsFinite(Score))
        {
            // treat NaN/Inf as disqualify
            continue;
        }

        if (Score > BestScore)
        {
            BestScore = Score;
            BestIndices.Reset();
            BestIndices.Add(Index);
        }
        else if (FMath::IsNearlyEqual(Score, BestScore))
        {
            BestIndices.Add(Index);
        }
    }

    if (BestIndices.Num() == 0)
    {
        return false;
    }

    int32 ChosenIndex = BestIndices[0];
    if (BestIndices.Num() > 1 && bRandomTieBreak)
    {
        ChosenIndex = BestIndices[FMath::RandRange(0, BestIndices.Num() - 1)];
    }

    OutAttack = AttackEntries[ChosenIndex];
    return true;
}

/**
 * BlueprintNativeEvent allows Blueprints to override the scoring heuristics.
 * Default implementation calls ScoreAttack_Implementation.
 *
 * @param Entry - candidate entry to score
 * @param Instigator - actor performing attack
 * @param Targets - array of targets to consider
 * @return a float score: higher = better. Return -INFINITY (or very low) to disqualify.
 */
float UMCS_AttackChooser::ScoreAttack_Implementation(const FMCS_AttackEntry& Entry, AActor* Instigator, const TArray<AActor*>& Targets) const
{
    float Score = Entry.SelectionWeight;

    // =====================================================
    // 1. GameplayTag filtering or preference
    // =====================================================
    if (RequiredAttackTag.IsValid())
    {
        const bool bMatches = Entry.AttackTag.MatchesTag(RequiredAttackTag);

        if (!bMatches)
        {
            if (!bPreferTagInsteadOfFilter)
            {
                // Strict filter: completely disqualify this attack
                return -TNumericLimits<float>::Max();
            }
            else
            {
                // Soft preference: lower the score by 50%
                Score *= 0.5f;
            }
        }
        else
        {
            // Reward for matching tag
            Score += 5.0f;
        }
    }

    // =====================================================
    // 2. Distance window scoring using RangeStart / RangeEnd
    // =====================================================
    if (Instigator && Targets.Num() > 0)
    {
        const FVector InstigatorLoc = Instigator->GetActorLocation();

        // Find the closest valid target
        AActor* ClosestTarget = nullptr;
        float ClosestDistSq = TNumericLimits<float>::Max();

        for (AActor* Target : Targets)
        {
            if (!IsValid(Target))
                continue;

            const float DistSq = FVector::DistSquared(InstigatorLoc, Target->GetActorLocation());
            if (DistSq < ClosestDistSq)
            {
                ClosestDistSq = DistSq;
                ClosestTarget = Target;
            }
        }

        if (ClosestTarget)
        {
            const float Dist = FMath::Sqrt(ClosestDistSq);

            // Score based on distance window
            if (Dist < Entry.RangeStart)
            {
                // Too close — penalize slightly
                const float UnderDist = Entry.RangeStart - Dist;
                Score -= UnderDist * 0.1f; // small penalty for being inside minimum range
            }
            else if (Dist > Entry.RangeEnd)
            {
                // Too far — disqualify if way out of range
                if (Dist > Entry.RangeEnd * 1.25f) // beyond 25% past range end
                {
                    return -TNumericLimits<float>::Max();
                }

                // Slight penalty if just beyond range
                const float OverDist = Dist - Entry.RangeEnd;
                Score -= OverDist * 0.2f;
            }
            else
            {
                // Inside valid range window
                const float RangeCenter = (Entry.RangeStart + Entry.RangeEnd) * 0.5f;
                const float WindowHalf = (Entry.RangeEnd - Entry.RangeStart) * 0.5f;
                const float DistanceFromCenter = FMath::Abs(Dist - RangeCenter);

                // Closer to the center of the range window = higher score
                const float ProximityFactor = 1.0f - (DistanceFromCenter / WindowHalf);
                const float RangeScore = FMath::Clamp(ProximityFactor, 0.f, 1.f) * 10.f;
                Score += RangeScore;
            }
        }
    }

    return Score;
}

/**
 * Basic filtering based on MaxTargetDistance and MaxTargetAngleDegrees.
 * Can be used as a quick pre-check before more complex scoring in ScoreAttack.
 * @param Entry - candidate entry to check
 * @param Instigator - actor performing attack
 * @param Targets - array of targets to consider
 * @return true if the entry passes basic filters, false otherwise
 */
bool UMCS_AttackChooser::IsEntryAllowedByBasicFilters(const FMCS_AttackEntry& Entry, AActor* Instigator, const TArray<AActor*>& Targets) const
{
    // If there's no instigator or no meaningful spatial data, we cannot filter on distance/angle.
    if (!Instigator)
    {
        // allow by default — ScoreAttack implementations should handle restrictions relying on actor data
        return true;
    }

    // If there are no targets, allow by default.
    if (Targets.Num() == 0)
    {
        return true;
    }

    // Check at least one target passes distance/angle constraints
    const FVector InstigatorLocation = Instigator->GetActorLocation();
    const FVector InstigatorForward = Instigator->GetActorForwardVector();

    for (TObjectPtr<AActor> TargetPtr : Targets)
    {
        if (!TargetPtr)
        {
            continue;
        }

        const FVector TargetLocation = TargetPtr->GetActorLocation();

        // Distance check
        if (MaxTargetDistance > 0.0f)
        {
            const float DistSq = FVector::DistSquared(InstigatorLocation, TargetLocation);
            if (DistSq > FMath::Square(MaxTargetDistance))
            {
                // too far for this target; check next target
                continue;
            }
        }

        // Angle check
        if (MaxTargetAngleDegrees > 0.0f && MaxTargetAngleDegrees < 180.0f)
        {
            const FVector ToTarget = (TargetLocation - InstigatorLocation).GetSafeNormal();
            const float CosAngle = FVector::DotProduct(InstigatorForward, ToTarget);
            const float AngleDeg = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(CosAngle, -1.0f, 1.0f)));
            if (AngleDeg > MaxTargetAngleDegrees)
            {
                // not facing enough; check next target
                continue;
            }
        }

        // If we reach here at least one target is acceptable
        return true;
    }

    // No targets passed the basic filters
    return false;
}

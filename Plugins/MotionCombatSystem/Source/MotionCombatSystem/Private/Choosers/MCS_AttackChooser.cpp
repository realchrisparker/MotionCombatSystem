/*
 * ========================================================================
 * Copyright © 2025 God's Studio
 * All Rights Reserved.
 *
 * Project: Motion Combat System
 * Author: Christopher D. Parker
 * Date: 10-14-2025
 * =============================================================================
 * MCS_AttackChooser.cpp
 * Implementation of UMCS_AttackChooser with modular scoring helpers.
 * Designers can reuse ComputeTagScore, ComputeDistanceScore, and
 * ComputeDirectionalScore directly inside Blueprints.
 * =============================================================================
 */

#include <Choosers/MCS_AttackChooser.h>
#include "GameFramework/Actor.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/UnrealMathUtility.h"

UMCS_AttackChooser::UMCS_AttackChooser()
{
    MaxTargetDistance = 2500.0f;
    MaxTargetAngleDegrees = 180.0f;
    bRandomTieBreak = true;
}

/* ==========================================================
 * Attack Selection
 * ========================================================== */
bool UMCS_AttackChooser::ChooseAttack(
    AActor* Instigator, const TArray<AActor*>& Targets, EMCS_AttackDirection DesiredDirection, FMCS_AttackEntry& OutAttack) const
{
    if (AttackEntries.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("[MCS_AttackChooser] No attacks to choose from."));
        return false;
    }

    float BestScore = -TNumericLimits<float>::Max();
    TArray<int32> BestIndices;

    for (int32 i = 0; i < AttackEntries.Num(); ++i)
    {
        const FMCS_AttackEntry& Entry = AttackEntries[i];
        if (!IsEntryAllowedByBasicFilters(Entry, Instigator, Targets))
            continue;

        const float Score = ScoreAttack(Entry, Instigator, Targets, DesiredDirection);
        if (!FMath::IsFinite(Score))
            continue;

        if (Score > BestScore)
        {
            BestScore = Score;
            BestIndices = { i };
        }
        else if (FMath::IsNearlyEqual(Score, BestScore))
        {
            BestIndices.Add(i);
        }
    }

    if (BestIndices.IsEmpty())
        return false;

    int32 ChosenIndex = BestIndices[0];
    if (BestIndices.Num() > 1 && bRandomTieBreak)
        ChosenIndex = BestIndices[FMath::RandRange(0, BestIndices.Num() - 1)];

    OutAttack = AttackEntries[ChosenIndex];
    return true;
}

/* ==========================================================
 * Core Scoring Logic
 * ========================================================== */
float UMCS_AttackChooser::ScoreAttack_Implementation(
    const FMCS_AttackEntry& Entry, AActor* Instigator, const TArray<AActor*>& Targets, EMCS_AttackDirection DesiredDirection) const
{
    const float BaseScore = Entry.SelectionWeight;
    const float TagScore = ComputeTagScore(Entry);
    const float DistanceScore = ComputeDistanceScore(Entry, Instigator, Targets);
    const float DirectionScore = ComputeDirectionalScore(Entry, DesiredDirection);

    return AggregateScore(BaseScore, TagScore, DistanceScore, DirectionScore);
}

/* ==========================================================
 * BlueprintPure Helper Implementations
 * ========================================================== */

 /**
  * Computes a score modifier based on tag filtering and preferences.
  * @param Entry The attack entry being evaluated.
  */
float UMCS_AttackChooser::ComputeTagScore(const FMCS_AttackEntry& Entry) const
{
    if (!RequiredAttackTag.IsValid())
        return 0.f;

    const bool bMatches = Entry.AttackTag.MatchesTag(RequiredAttackTag);
    if (!bMatches)
    {
        if (!bPreferTagInsteadOfFilter)
            return -TNumericLimits<float>::Max(); // disqualify
        return Entry.SelectionWeight * -0.5f;     // soft penalty
    }

    return 5.f; // reward for match
}

/**
 * Computes a score modifier based on distance window.
 * @param Entry The attack entry being evaluated.
 * @param Instigator The actor performing the attack.
 * @param Targets The potential targets of the attack.
 */
float UMCS_AttackChooser::ComputeDistanceScore(const FMCS_AttackEntry& Entry, AActor* Instigator, const TArray<AActor*>& Targets) const
{
    if (!Instigator || Targets.IsEmpty())
        return 0.f;

    AActor* ClosestTarget = nullptr;
    float ClosestDistSq = TNumericLimits<float>::Max();
    const FVector InstigatorLoc = Instigator->GetActorLocation();

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

    if (!ClosestTarget)
        return 0.f;

    const float Dist = FMath::Sqrt(ClosestDistSq);
    if (Dist < Entry.RangeStart)
        return -(Entry.RangeStart - Dist) * 0.1f;

    if (Dist > Entry.RangeEnd)
    {
        if (Dist > Entry.RangeEnd * 1.25f)
            return -TNumericLimits<float>::Max();
        return -(Dist - Entry.RangeEnd) * 0.2f;
    }

    // Inside range window
    const float Center = (Entry.RangeStart + Entry.RangeEnd) * 0.5f;
    const float HalfWindow = (Entry.RangeEnd - Entry.RangeStart) * 0.5f;
    const float Offset = FMath::Abs(Dist - Center);
    const float ProximityFactor = 1.0f - (Offset / HalfWindow);
    return FMath::Clamp(ProximityFactor, 0.f, 1.f) * 10.f;
}

/**
 * Computes a score modifier based on desired attack direction.
 * @param Entry The attack entry being evaluated.
 * @param DesiredDirection The desired attack direction.
 */
float UMCS_AttackChooser::ComputeDirectionalScore(const FMCS_AttackEntry& Entry, EMCS_AttackDirection DesiredDirection) const
{
    if (Entry.AttackDirection == EMCS_AttackDirection::Omni)
        return 5.f;
    if (Entry.AttackDirection == DesiredDirection)
        return 10.f;

    // Opposite penalties
    if ((Entry.AttackDirection == EMCS_AttackDirection::Forward && DesiredDirection == EMCS_AttackDirection::Backward) ||
        (Entry.AttackDirection == EMCS_AttackDirection::Backward && DesiredDirection == EMCS_AttackDirection::Forward) ||
        (Entry.AttackDirection == EMCS_AttackDirection::Left && DesiredDirection == EMCS_AttackDirection::Right) ||
        (Entry.AttackDirection == EMCS_AttackDirection::Right && DesiredDirection == EMCS_AttackDirection::Left))
    {
        return -10.f;
    }

    return 0.f;
}

/**
 * Aggregates individual score components into a final score.
 * Designers can override this in Blueprint to weight components differently.
 * @param BaseScore The base selection weight of the attack.
 * @param TagScore The score modifier from tag filtering.
 * @param DistanceScore The score modifier from distance evaluation.
 * @param DirectionScore The score modifier from directional matching.
 */
float UMCS_AttackChooser::AggregateScore(float BaseScore, float TagScore, float DistanceScore, float DirectionScore) const
{
    // Designers can override this in Blueprint to weight components differently.
    return BaseScore + TagScore + DistanceScore + DirectionScore;
}

/* ==========================================================
 * Basic Entry Filtering
 * ==========================================================
 */

 /**
  * Is entry allowed by basic filters (distance & angle).
  */
bool UMCS_AttackChooser::IsEntryAllowedByBasicFilters(const FMCS_AttackEntry& Entry, AActor* Instigator, const TArray<AActor*>& Targets) const
{
    if (!Instigator)
        return true;
    if (Targets.IsEmpty())
        return true;

    const FVector InstigatorLoc = Instigator->GetActorLocation();
    const FVector InstigatorForward = Instigator->GetActorForwardVector();

    for (AActor* Target : Targets)
    {
        if (!IsValid(Target))
            continue;

        const FVector TargetLoc = Target->GetActorLocation();
        const float DistSq = FVector::DistSquared(InstigatorLoc, TargetLoc);
        if (MaxTargetDistance > 0.f && DistSq > FMath::Square(MaxTargetDistance))
            continue;

        if (MaxTargetAngleDegrees > 0.f && MaxTargetAngleDegrees < 180.f)
        {
            const FVector ToTarget = (TargetLoc - InstigatorLoc).GetSafeNormal();
            const float CosAngle = FVector::DotProduct(InstigatorForward, ToTarget);
            const float AngleDeg = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(CosAngle, -1.0f, 1.0f)));
            if (AngleDeg > MaxTargetAngleDegrees)
                continue;
        }

        return true; // Passed
    }

    return false;
}

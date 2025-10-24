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
 * Attack Selection & Scoring Implementations
 * ========================================================== */

 /*
 * Attack Selection
 */
bool UMCS_AttackChooser::ChooseAttack(
    AActor* Instigator,
    const TArray<AActor*>& Targets,
    EMCS_AttackDirection DesiredDirection,
    const FMCS_AttackSituation& CurrentSituation,
    FMCS_AttackEntry& OutAttack) const
{
    if (AttackEntries.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("[MCS_AttackChooser] No attacks to choose from."));
        return false;
    }

#if WITH_EDITORONLY_DATA || UE_BUILD_DEVELOPMENT
    ClearDebugScores();
#endif

    float BestScore = -TNumericLimits<float>::Max();
    TArray<int32> BestIndices;

    for (int32 i = 0; i < AttackEntries.Num(); ++i)
    {
        const FMCS_AttackEntry& Entry = AttackEntries[i];
        if (!IsEntryAllowedByBasicFilters(Entry, Instigator, Targets))
            continue;

        // Pass CurrentSituation into the scoring function
        const float Score = ScoreAttack(Entry, Instigator, Targets, DesiredDirection, CurrentSituation);
        if (!FMath::IsFinite(Score))
            continue;

#if WITH_EDITORONLY_DATA || UE_BUILD_DEVELOPMENT
        FMCS_DebugAttackScore DebugEntry;
        DebugEntry.AttackName = Entry.AttackName;
        DebugEntry.BaseScore = Entry.SelectionWeight;
        DebugEntry.TagScore = ComputeTagScore(Entry);
        DebugEntry.DistanceScore = ComputeDistanceScore(Entry, Instigator, Targets);
        DebugEntry.DirectionScore = ComputeDirectionalScore(Entry, DesiredDirection);
        DebugEntry.SituationScore = ComputeSituationScore(Entry, CurrentSituation);
        DebugEntry.TotalScore = DebugEntry.BaseScore + DebugEntry.TagScore + DebugEntry.DistanceScore + DebugEntry.DirectionScore + DebugEntry.SituationScore;
        DebugEntry.Notes = FString::Printf(TEXT("Tag:%+.1f Dist:%+.1f Dir:%+.1f Sit:%+.1f"),
            DebugEntry.TagScore, DebugEntry.DistanceScore, DebugEntry.DirectionScore, DebugEntry.SituationScore);
        DebugScores.Add(DebugEntry);
#endif

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

#if WITH_EDITORONLY_DATA || UE_BUILD_DEVELOPMENT
    // Mark the winning entry
    for (FMCS_DebugAttackScore& Info : DebugScores)
    {
        Info.bWasChosen = (Info.AttackName == AttackEntries[ChosenIndex].AttackName);
    }
#endif

    OutAttack = AttackEntries[ChosenIndex];
    return true;
}


/*
 * Core Scoring Logic
 */
float UMCS_AttackChooser::ScoreAttack_Implementation(
    const FMCS_AttackEntry& Entry,
    AActor* Instigator,
    const TArray<AActor*>& Targets,
    EMCS_AttackDirection DesiredDirection,
    const FMCS_AttackSituation& CurrentSituation) const
{
    const float BaseScore = Entry.SelectionWeight;
    const float TagScore = ComputeTagScore(Entry);
    const float DistanceScore = ComputeDistanceScore(Entry, Instigator, Targets);
    const float DirectionScore = ComputeDirectionalScore(Entry, DesiredDirection);
    const float SituationScoreValue = ComputeSituationScore(Entry, CurrentSituation);

    return AggregateScore(BaseScore, TagScore, DistanceScore, DirectionScore, SituationScoreValue);
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

float UMCS_AttackChooser::ComputeSituationScore(const FMCS_AttackEntry& Entry, const FMCS_AttackSituation& CurrentSituation) const
{
    float Score = 0.f;

    switch (Entry.AttackSituation)
    {
        case EMCS_AttackSituations::Grounded:
            if (CurrentSituation.bIsGrounded) Score += 10.f;
            else if (CurrentSituation.bIsInAir) Score -= 10.f;
            break;

        case EMCS_AttackSituations::Airborne:
            if (CurrentSituation.bIsInAir) Score += 15.f;
            else Score -= 10.f;
            break;

        case EMCS_AttackSituations::Running:
            if (CurrentSituation.bIsRunning) Score += 10.f;
            break;

        case EMCS_AttackSituations::Crouching:
            if (CurrentSituation.bIsCrouching) Score += 10.f;
            break;

        case EMCS_AttackSituations::Counter:
            if (CurrentSituation.bIsCountering) Score += 20.f;
            break;

        case EMCS_AttackSituations::Parry:
            if (CurrentSituation.bIsParrying) Score += 25.f;
            break;

        case EMCS_AttackSituations::Riposte:
            if (CurrentSituation.bIsRiposting) Score += 30.f;
            break;

        case EMCS_AttackSituations::Finisher:
            if (CurrentSituation.bIsFinishing) Score += 25.f;
            break;

        case EMCS_AttackSituations::Any:
        default:
            Score += 5.f; // always somewhat valid
            break;
    }

    // ----------------------------------------------------------
    // Extended quantitative condition checks (designer-defined)
    // ----------------------------------------------------------
    for (const FMCS_AttackCondition& Condition : Entry.ConditionalChecks)
    {
        const float CurrentValue = QueryAttributeValue(Condition.AttributeName, CurrentSituation);

        bool bPass = false;
        switch (Condition.Comparison)
        {
            case EMCS_ComparisonMethod::Equal:          bPass = FMath::IsNearlyEqual(CurrentValue, Condition.Threshold, 0.01f); break;
            case EMCS_ComparisonMethod::NotEqual:       bPass = !FMath::IsNearlyEqual(CurrentValue, Condition.Threshold, 0.01f); break;
            case EMCS_ComparisonMethod::Greater:        bPass = CurrentValue > Condition.Threshold; break;
            case EMCS_ComparisonMethod::Less:           bPass = CurrentValue < Condition.Threshold; break;
            case EMCS_ComparisonMethod::GreaterOrEqual: bPass = CurrentValue >= Condition.Threshold; break;
            case EMCS_ComparisonMethod::LessOrEqual:    bPass = CurrentValue <= Condition.Threshold; break;
        }

        if (bPass)
            Score += Condition.Weight;
    }

    return Score;
}

/**
 * Aggregates individual score components into a final score.
 * Designers can override this in Blueprint to weight components differently.
 * @param BaseScore The base selection weight of the attack.
 * @param TagScore The score modifier from tag filtering.
 * @param DistanceScore The score modifier from distance evaluation.
 * @param DirectionScore The score modifier from directional matching.
 */
float UMCS_AttackChooser::AggregateScore(float BaseScore, float TagScore, float DistanceScore, float DirectionScore, float SituationScore) const
{
    return BaseScore + TagScore + DistanceScore + DirectionScore + SituationScore;
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

/**
 * Queries numeric attributes from the current situation context.
 * Extend this function to expose new values (e.g., Stamina, Altitude, etc.)
 */
float UMCS_AttackChooser::QueryAttributeValue(FName Attribute, const FMCS_AttackSituation& Situation) const
{
    if (Attribute == "Speed")     return Situation.Speed;
    if (Attribute == "Altitude")  return Situation.Altitude;
    if (Attribute == "Stamina")   return Situation.Stamina;
    if (Attribute == "Health")    return Situation.HealthPercent;
    // Extend as needed
    return 0.f;
}

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
void UMCS_AttackChooser::ClearDebugScores() const
{
    DebugScores.Reset();
}
#endif
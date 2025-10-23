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
 * Date: 10-18-2025
 * =============================================================================
 * MCS_CombatHitReactionComponent.cpp
 *
 * Implementation for the combat hit reaction component.
 * Future versions will handle responding to damage and playing
 * appropriate hit reaction animations or montages.
 */

#include <Components/MCS_CombatHitReactionComponent.h>

 // Constructor
UMCS_CombatHitReactionComponent::UMCS_CombatHitReactionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UMCS_CombatHitReactionComponent::BeginPlay()
{
    Super::BeginPlay();
}

/**
 * Performs a hit reaction based on the specified hit direction.
 * @param HitDirection The direction of the hit (e.g., "Hit.Back", "Hit.Forward", etc.).
 * @param PlayRate The play rate for the hit reaction montage.
*/
void UMCS_CombatHitReactionComponent::PerformHitReaction(EMCS_Direction Direction, EPGAS_HitSeverity Severity)
{
    // if (!CachedOwnerCharacter)
    // {
    //     UE_LOG(LogTemp, Warning, TEXT("HitReactionComponent: No owner character."));
    //     return;
    // }

    // // Find the best reaction
    // const FPGAS_HitReaction* Reaction = FindReaction(Direction, Severity);

    // if (!Reaction)
    // {
    //     UE_LOG(LogTemp, Warning, TEXT("HitReactionComponent: No matching hit reaction found."));
    //     return;
    // }

    // // Activate ability or play montage
    // // Abilities take precedence over montages
    // if (Reaction->AbilityTag.IsValid())
    // {
    //     // If the reaction has an ability tag, try to activate that ability by event
    //     UPGAS_HitReactionDataObject* DataObj = NewObject<UPGAS_HitReactionDataObject>();
    //     DataObj->Data = *Reaction;

    //     // Build event data
    //     FGameplayEventData EventData;
    //     EventData.EventTag = Reaction->AbilityTag; // ability listens for this
    //     EventData.OptionalObject = DataObj;

    //     UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(CachedOwnerCharacter, Reaction->AbilityTag, EventData);
    // }
    // else if (Reaction->Montage) // Play the montage if found
    // {
    //     PlayMontageInternal(Reaction->Montage, Reaction->PlayRate);
    // }
    // else
    // {
    //     UE_LOG(LogTemp, Warning, TEXT("HitReactionComponent: No matching hit reaction found for %d + %d"),
    //         (int32)Direction, (int32)Severity);
    // }
}

/**
 * Finds the best matching reaction based on direction and severity.
 * @param Direction The hit direction.
 * @param Severity The hit severity.
 * @return The best matching hit reaction, or nullptr if none found.
 */
const FMCS_HitReaction* UMCS_CombatHitReactionComponent::FindReaction(EMCS_Direction Direction, EPGAS_HitSeverity Severity) const
{
    // // First look for exact match
    // for (const FMCS_HitReaction& Reaction : HitReactions)
    // {
    //     if (Reaction.Direction == Direction && Reaction.Severity == Severity)
    //     {
    //         return &Reaction;
    //     }
    // }

    // // If none found, fallback: try matching only by severity
    // for (const FPGAS_HitReaction& Reaction : HitReactions)
    // {
    //     if (Reaction.Severity == Severity)
    //     {
    //         return &Reaction;
    //     }
    // }

    // // If still none found, fallback: try matching only by direction
    // for (const FPGAS_HitReaction& Reaction : HitReactions)
    // {
    //     if (Reaction.Direction == Direction)
    //     {
    //         return &Reaction;
    //     }
    // }

    // Nothing found
    return nullptr;
}

/**
 * Helper: Plays a montage on the owning actor's mesh if valid
 * @param Montage The montage to play
 * @param InPlayRate The play rate for the montage
 */
void UMCS_CombatHitReactionComponent::PlayMontageInternal(UAnimMontage* Montage, float InPlayRate)
{
    // if (Montage && CachedOwnerCharacter)
    // {
    //     if (UAnimInstance* AnimInstance = CachedOwnerCharacter->GetMesh()->GetAnimInstance())
    //     {
    //         AnimInstance->Montage_Play(Montage, InPlayRate);
    //     }
    // }
}

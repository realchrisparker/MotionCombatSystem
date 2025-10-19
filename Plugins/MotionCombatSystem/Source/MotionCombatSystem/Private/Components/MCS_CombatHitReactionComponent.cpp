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

    // TODO: Hook into damage or hit events here when implemented.
}

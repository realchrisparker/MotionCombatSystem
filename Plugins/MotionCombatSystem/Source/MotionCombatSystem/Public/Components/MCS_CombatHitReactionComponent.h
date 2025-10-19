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
 * MCS_CombatHitReactionComponent.h
 *
 * Component responsible for handling hit reactions (e.g. playing
 * reaction montages or effects) when the owning actor takes damage or is hit.
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MCS_CombatHitReactionComponent.generated.h"

UCLASS(Blueprintable, ClassGroup = (MotionCombatSystem), meta = (BlueprintSpawnableComponent, DisplayName = "Motion Combat Hit Reaction Component"))
class MOTIONCOMBATSYSTEM_API UMCS_CombatHitReactionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Constructor
    UMCS_CombatHitReactionComponent();

protected:
    // Called when the game starts
    virtual void BeginPlay() override;
};

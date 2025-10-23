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
#include <Structs/MCS_HitReaction.h>
#include "MCS_CombatHitReactionComponent.generated.h"

UCLASS(Blueprintable, ClassGroup = (MotionCombatSystem), meta = (BlueprintSpawnableComponent, DisplayName = "Motion Combat Hit Reaction Component"))
class MOTIONCOMBATSYSTEM_API UMCS_CombatHitReactionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Constructor
    UMCS_CombatHitReactionComponent();

    /*
    * Functions
    */

    /**
     * Performs a hit reaction based on the specified hit direction.
     * @param HitDirection The direction of the hit (e.g., "Hit.Back", "Hit.Forward", etc.).
     * @param PlayRate The play rate for the hit reaction montage.
    */
    UFUNCTION(BlueprintCallable, Category = "Hit Reaction")
    void PerformHitReaction(EMCS_Direction Direction, EPGAS_HitSeverity Severity);

    /*
    * Properties
    */

    /** DataTable containing FPGAS_HitReaction definitions */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Core", meta = (DisplayName = "Hit Reaction Data Table"))
    TObjectPtr<UDataTable> HitReactionDataTable;

    // /** All available hit reactions (configured in editor) */
    // UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction", meta = (DisplayName = "Hit Reactions"))
    // TArray<FPGAS_HitReaction> HitReactions;

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

private:

    /*
    * Functions
    */

    /** Helper: Plays a montage on the owning actor's mesh if valid */
    void PlayMontageInternal(UAnimMontage* Montage, float InPlayRate = 1.0f);

    /**
     * Finds the best matching reaction based on direction and severity.
     * @param Direction The hit direction.
     * @param Severity The hit severity.
     * @return The best matching hit reaction, or nullptr if none found.
     */
    const FMCS_HitReaction* FindReaction(EMCS_Direction Direction, EPGAS_HitSeverity Severity) const;
};

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
 * MCS_CombatHitboxComponent.h
 * Simple socket-driven hitbox (StartSocket → EndSocket).
 * Tick-based sphere sweep while detection is active.
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include <Structs/MCS_AttackEntry.h>
#include <Structs/MCS_AttackHitbox.h>
#include "MCS_CombatHitboxComponent.generated.h"


/*
 * Delegates
 */

// Delegate for hit events
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FMCS_OnSimpleHitSignature, AActor*, HitActor, const FHitResult&, HitResult, FMCS_AttackEntry, AttackEntry);


/**
 * Simple socket-driven hitbox (StartSocket → EndSocket).
 * Tick-based sphere sweep while detection is active.
 */
UCLASS(BlueprintType, ClassGroup = (MotionCombatSystem), meta = (BlueprintSpawnableComponent, DisplayName = "Motion Combat System Hitbox Component"))
class MOTIONCOMBATSYSTEM_API UMCS_CombatHitboxComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMCS_CombatHitboxComponent();

    /*
     * Functions
     */
    
    /** Start hit detection (begins ticking sweeps). */
    UFUNCTION(BlueprintCallable, Category = "MCS|Hitbox")
    void StartHitDetection(const FMCS_AttackEntry& Attack, const FMCS_AttackHitbox& Hitbox);

    /** Stop hit detection (stops ticking sweeps). */
    UFUNCTION(BlueprintCallable, Category = "MCS|Hitbox")
    void StopHitDetection();

    /** Is currently detecting hits? */
    UFUNCTION(BlueprintPure, Category = "MCS|Hitbox")
    bool IsDetecting() const { return bIsDetecting; }

    /**
     * Clear the list of already hit actors for the current swing.
     * Useful to allow multi-hit within a combo.
     */
    UFUNCTION(BlueprintCallable, Category = "MCS|Hitbox")
    void ResetAlreadyHit()
    {
        AlreadyHitActors.Reset();
    }

    /*
     * Properties
     */

     // Number of substeps to interpolate between frames
    UPROPERTY(EditAnywhere, Category = "MCS|Hitbox")
    int32 SubstepCount = 2; // 2–4 is usually plenty

    /** Broadcast when a hit is registered. */
    UPROPERTY(BlueprintAssignable, Category = "MCS|Hitbox")
    FMCS_OnSimpleHitSignature OnHitboxHit;

protected:
    /*
     * Functions
     */

    /** Called when the game starts */
    virtual void BeginPlay() override;

    /** Called every frame */
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    /*
     * Functions
     */

    USkeletalMeshComponent* ResolveMesh() const
    {
        if (const AActor* Owner = GetOwner())
        {
            return Owner->FindComponentByClass<USkeletalMeshComponent>();
        }

        return nullptr;
    }

    void PerformSweep();

    /*
     * Properties
     */

    // Is currently detecting hits?
    bool bIsDetecting = false;

    // Cached from AttackType when StartHitDetection is called
    FMCS_AttackEntry ActiveAttack;

    // Cached hitbox
    FMCS_AttackHitbox ActiveHitbox;

    // Previous frame socket positions
    FVector PrevStartLoc = FVector::ZeroVector;
    FVector PrevEndLoc = FVector::ZeroVector;

    // Prevent hitting same actor multiple times in one swing
    TSet<TWeakObjectPtr<AActor>> AlreadyHitActors;
};

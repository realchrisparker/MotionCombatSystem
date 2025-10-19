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
 * UAnimNotifyState_MCSHitboxWindow.h
 * Defines a notify window that stores hitbox configuration for the core component to manage.
 */

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include <Structs/MCS_AttackHitbox.h>
#include "AnimNotifyState_MCSHitboxWindow.generated.h"


 /*
  * Delegates
 */

// Delegate called when the notify begins
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNotifyBegin, FMCS_AttackHitbox&, AttackEntry);

// Delegate called when the notify ends
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNotifyEnd, FMCS_AttackHitbox&, AttackEntry);

 /**
  * Designer-friendly notify window used to mark when a hitbox should be active.
  */
UCLASS(Blueprintable, ClassGroup=(MotionCombatSystem), meta = (DisplayName = "Motion Combat Hitbox Window"))
class MOTIONCOMBATSYSTEM_API UAnimNotifyState_MCSHitboxWindow : public UAnimNotifyState
{
    GENERATED_BODY()

public:

    /*
     * Properties
     */

    /** Hitbox configuration for this notify window (designer-defined). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Hitbox")
    FMCS_AttackHitbox Hitbox;

    /** Optional label for clarity in animation timelines. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Debug")
    FName DebugLabel = NAME_None;

    /** Whether to draw debug visualization during this window (handled by the core). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Debug")
    bool bDebugDraw = true;

    //--------------
    // Events
    //--------------

    // Delegate: called on notify begin
    UPROPERTY(BlueprintAssignable, Category = "Notify")
    FOnNotifyBegin OnNotifyBegin;

    // Delegate: called on notify end
    UPROPERTY(BlueprintAssignable, Category = "Notify")
    FOnNotifyEnd OnNotifyEnd;
    
    /*
     * Functions
    */
    
    // Begin notification logic
    virtual void NotifyBegin(
        USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override
    {
        if (!MeshComp) return;
        OnNotifyBegin.Broadcast(Hitbox); // Broadcast the begin event
    }

    // End notification logic
    virtual void NotifyEnd(
        USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override
    {
        if (!MeshComp) return;
        OnNotifyEnd.Broadcast(Hitbox); // Broadcast the end event
    }
};

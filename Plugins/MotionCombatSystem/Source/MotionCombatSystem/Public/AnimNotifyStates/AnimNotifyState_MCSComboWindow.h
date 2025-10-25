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
 * AnimNotifyState_MCSComboWindow.h
 * Defines a notify window that stores hitbox configuration for the core component to manage.
 */

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_MCSComboWindow.generated.h"


 /*
  * Delegates
 */

 // Delegate called when the notify begins
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNotifyBegin2);

// Delegate called when the notify ends
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNotifyEnd2);

/**
 * Designer-friendly notify window used to mark when a combo should be active.
 */
UCLASS(Blueprintable, ClassGroup = (MotionCombatSystem), meta = (DisplayName = "Motion Combat Combo Window"))
class MOTIONCOMBATSYSTEM_API UAnimNotifyState_MCSComboWindow : public UAnimNotifyState
{
    GENERATED_BODY()

public:

    /*
     * Properties
     */

    /** Optional label for clarity in animation timelines. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Debug")
    FName DebugLabel = NAME_None;

    //--------------
    // Events
    //--------------

    // Delegate: called on notify begin
    UPROPERTY(BlueprintAssignable, Category = "Notify")
    FOnNotifyBegin2 OnNotifyBegin;

    // Delegate: called on notify end
    UPROPERTY(BlueprintAssignable, Category = "Notify")
    FOnNotifyEnd2 OnNotifyEnd;

    /*
     * Functions
    */

    // Begin notification logic
    virtual void NotifyBegin(
        USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override
    {
        if (!MeshComp) return;
        OnNotifyBegin.Broadcast(); // Broadcast the begin event
    }

    // End notification logic
    virtual void NotifyEnd(
        USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override
    {
        if (!MeshComp) return;
        OnNotifyEnd.Broadcast(); // Broadcast the end event
    }
};

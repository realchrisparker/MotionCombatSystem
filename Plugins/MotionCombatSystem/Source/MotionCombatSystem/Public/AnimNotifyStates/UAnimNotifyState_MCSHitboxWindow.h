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
#include "UAnimNotifyState_MCSHitboxWindow.generated.h"

 /**
  * Designer-friendly notify window used to mark when a hitbox should be active.
  *
  * NOTE: This notify does *not* activate hit detection by itself. The
  * UMCS_CombatCoreComponent is responsible for listening for these events
  * and triggering UMCS_CombatHitboxComponent start/stop calls.
  */
UCLASS(Blueprintable, ClassGroup=(MotionCombatSystem), meta = (DisplayName = "Motion Combat Hitbox Window"))
class MOTIONCOMBATSYSTEM_API UAnimNotifyState_MCSHitboxWindow : public UAnimNotifyState
{
    GENERATED_BODY()

public:

    /** Hitbox configuration for this notify window (designer-defined). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Hitbox")
    FMCS_AttackHitbox Hitbox;

    /** Optional label for clarity in animation timelines. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Debug")
    FName DebugLabel = NAME_None;

    /** Whether to draw debug visualization during this window (handled by the core). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Debug")
    bool bDebugDraw = true;

    // Intentionally empty: handled by core component logic
    virtual void NotifyBegin(USkeletalMeshComponent*, UAnimSequenceBase*, float) override {}
    virtual void NotifyEnd(USkeletalMeshComponent*, UAnimSequenceBase*) override {}
};

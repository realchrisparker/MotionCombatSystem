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
 * Date: 10-13-2025
 * =============================================================================
 * MCS_CombatTargetInterface.h
 * Interface that marks an actor as being targetable by the Motion Combat System.
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MCS_CombatTargetInterface.generated.h"

 // This macro creates the interface’s UClass type
UINTERFACE(Blueprintable, meta = (DisplayName = "Motion Combat System Target Interface"))
class MOTIONCOMBATSYSTEM_API UMCS_CombatTargetInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 * Interface for actors that can be targeted by the Motion Combat System.
 */
class MOTIONCOMBATSYSTEM_API IMCS_CombatTargetInterface
{
    GENERATED_BODY()

public:
    /** Whether this actor can currently be targeted (true = valid target). */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat Target")
    bool CanBeTargeted() const;
    virtual bool CanBeTargeted_Implementation() const { return true; }
};

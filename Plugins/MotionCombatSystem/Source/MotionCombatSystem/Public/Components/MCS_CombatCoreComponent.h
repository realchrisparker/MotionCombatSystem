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
 * MCS_CoreComponent.h
 * Core combat component that coordinates attack selection, target acquisition,
 * and DataTable-driven attack loading.
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include <Structs/MCS_AttackEntry.h>
#include <SubSystems/MCS_TargetingSubsystem.h>
#include <Choosers/MCS_AttackChooser.h>
#include "MCS_CombatCoreComponent.generated.h"

UCLASS(Blueprintable, ClassGroup = (MotionCombat), meta = (BlueprintSpawnableComponent, DisplayName = "Motion Combat System Core Component"))
class MOTIONCOMBATSYSTEM_API UMCS_CombatCoreComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Constructor
    UMCS_CombatCoreComponent();

protected:
    virtual void BeginPlay() override;

public:

    /*
     * Properties
     */

    /** DataTable containing FMCS_AttackEntry definitions */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Core")
    TObjectPtr<UDataTable> AttackDataTable;

    /** Attack chooser instance (instantiated in BeginPlay if not provided) */
    UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, Category = "MCS|Core")
    TObjectPtr<UMCS_AttackChooser> AttackChooser;

    /** Cached reference to the world’s targeting subsystem */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MCS|Core")
    TObjectPtr<UMCS_TargetingSubsystem> TargetingSubsystem;

    /** Currently selected attack (if any) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MCS|Core")
    FMCS_AttackEntry CurrentAttack;

    /** Debug toggle */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Core|Debug")
    bool bDebugCombat = true;

    /*
     * Functions
     */

     /** Loads all FMCS_AttackEntry rows from the DataTable into the AttackChooser */
    UFUNCTION(BlueprintCallable, Category = "MCS|Core")
    void LoadAttacksFromDataTable();

    /** Chooses an appropriate attack using AttackChooser and available targets, filtered by type */
    UFUNCTION(BlueprintCallable, Category = "MCS|Core")
    bool SelectAttack(EFMCS_AttackType DesiredType);

    /** Combined helper: selects and executes an attack of the given type (e.g. Light/Heavy) */
    UFUNCTION(BlueprintCallable, Category = "MCS|Core")
    void PerformAttack(EFMCS_AttackType DesiredType);

    /** Gets the closest valid target via TargetingSubsystem */
    UFUNCTION(BlueprintCallable, Category = "MCS|Core")
    AActor* GetClosestTarget(float MaxRange = 2500.f) const;

private:
    /** Utility to get the owning actor safely */
    AActor* GetOwnerActor() const;
};

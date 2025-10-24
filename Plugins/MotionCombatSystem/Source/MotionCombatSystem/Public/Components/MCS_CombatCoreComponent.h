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
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include <Structs/MCS_AttackEntry.h>
#include <Structs/MCS_AttackSetData.h>
#include <SubSystems/MCS_TargetingSubsystem.h>
#include <Choosers/MCS_AttackChooser.h>
#include <AnimNotifyStates/AnimNotifyState_MCSHitboxWindow.h>
#include <Components/MCS_CombatHitboxComponent.h>
#include "MCS_CombatCoreComponent.generated.h"


/*
 * Delegates
 */

// Delegate broadcast when the target list is updated
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTargetingUpdatedSignature, const TArray<FMCS_TargetInfo>&, NewTargetList, int32, NumTargets);


/**
 * Core combat component that coordinates attack selection, target acquisition, and DataTable-driven attack loading.
 */
UCLASS(Blueprintable, ClassGroup = (MotionCombatSystem), meta = (BlueprintSpawnableComponent, DisplayName = "Motion Combat System Core Component"))
class MOTIONCOMBATSYSTEM_API UMCS_CombatCoreComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Constructor
    UMCS_CombatCoreComponent();

    /*
     * Properties
     */

    /** Holds multiple attack sets, each keyed by a GameplayTag (e.g. "Weapon.Sword", "Stance.Air") */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Core", meta = (DisplayName = "Attack Sets"))
    TMap<FGameplayTag, FMCS_AttackSetData> AttackSets;

    // /**
    //  * Enables debug logging and visual overlay output for the Motion Combat System.
    //  * When true, the system will display attack selection scores and debug information
    //  * in the viewport (if available) and log detailed info to the console.
    //  */
    // UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Core|Debug", meta = (DisplayName = "Enable Debug Visualization"))
    // bool bDebug = false;

    /** Blueprint Event triggered whenever the TargetingSubsystem's target list is updated */
    UPROPERTY(BlueprintAssignable, Category = "MCS|Events", meta = (DisplayName = "On Targeting Updated"))
    FOnTargetingUpdatedSignature OnTargetingUpdated;

    /*
     * Functions
     */

    /**
     * Chooses an appropriate attack using AttackChooser and available targets, filtered by type
     * @param DesiredType - type of attack to select
     * @param DesiredDirection - direction of the attack in world space
     */
    UFUNCTION(BlueprintCallable, Category = "MCS|Core",
        meta = (DisplayName = "Select Attack",
            ToolTip = "Selects an attack entry without executing it. Use only if you need to preview or queue attacks manually."))
    bool SelectAttack(EMCS_AttackType DesiredType, EMCS_AttackDirection DesiredDirection, const FMCS_AttackSituation& CurrentSituation);

    /**
     * Plays the selected attack's montage if valid
     * @param DesiredType - type of attack to perform
     * @param DesiredDirection - direction of the attack in world space
     */
    UFUNCTION(BlueprintCallable, Category = "MCS|Core",
        meta = (DisplayName = "Perform Attack", ToolTip = "Selects and executes an attack. You do not need to call SelectAttack first."))
    void PerformAttack(EMCS_AttackType DesiredType, EMCS_AttackDirection DesiredDirection, const FMCS_AttackSituation& CurrentSituation);

    /** Gets the closest valid target via TargetingSubsystem */
    UFUNCTION(BlueprintCallable, Category = "MCS|Core", meta = (DisplayName = "Get Closest Target"))
    AActor* GetClosestTarget(float MaxRange = 2500.f) const;

    /**
     * Utility to convert 2D movement input into an EMCS_AttackDirection enum value
     * @param MoveInput - 2D movement input vector (X=Forward/Backward, Y=Left/Right)
     * @return Corresponding EMCS_AttackDirection value
    */
    UFUNCTION(BlueprintPure, Category = "MCS|Core", meta = (DisplayName = "Get Attack Direction"))
    EMCS_AttackDirection GetAttackDirection(const FVector2D& MoveInput) const;

    /**
     * Sets the active attack DataTable using a gameplay tag.
     * If successful, returns true and updates the AttackChooser.
     */
    UFUNCTION(BlueprintCallable, Category = "MCS|Core", meta = (DisplayName = "Set Active Attack Set"))
    bool SetActiveAttackSet(const FGameplayTag& NewAttackSetTag);

    /**
     * Gets the currently active attack DataTable (if any).
     */
    UFUNCTION(BlueprintPure, Category = "MCS|Core", meta = (DisplayName = "Get Active Attack Table"))
    UDataTable* GetActiveAttackTable() const;

    /**
     * Gets the currently selected attack (if any).
     */
    UFUNCTION(BlueprintPure, Category = "MCS|Core", meta = (DisplayName = "Get Current Attack"))
    FMCS_AttackEntry GetCurrentAttack() const { return CurrentAttack; }

#if WITH_EDITORONLY_DATA
    /**
     * Draws the Motion Combat System debug overlay.
     * Call from your PlayerController::DrawHUD() or custom AHUD::DrawHUD().
     */
    void DrawDebugOverlay(FCanvas* Canvas, float& Y) const;
#endif

protected:
    virtual void BeginPlay() override;

private:
    
    /*
     * Properties
     */
    
    /** Utility to get the owning actor safely */
    AActor* GetOwnerActor() const;

    /** DataTable containing FMCS_AttackEntry definitions */
    UPROPERTY()
    TObjectPtr<UDataTable> AttackDataTable;

    /** Cached reference to the world’s targeting subsystem */
    UPROPERTY()
    TObjectPtr<UMCS_TargetingSubsystem> TargetingSubsystem;

    /** Currently selected attack (if any) */
    UPROPERTY()
    FMCS_AttackEntry CurrentAttack;

    /** The currently active attack set tag */
    UPROPERTY()
    FGameplayTag ActiveAttackSetTag;

    /*
     * Functions
     */

    // Handler for TargetingSubsystem target updates
    UFUNCTION()
    void HandleTargetsUpdated(const TArray<FMCS_TargetInfo>& NewTargets, int32 NewTargetCount);

    /** Cached list of hitbox window data parsed from the current montage */
    TArray<FMCS_AttackHitbox> CachedHitboxWindows;

    /** Cached pointer to owner’s hitbox component */
    TObjectPtr<UMCS_CombatHitboxComponent> CachedHitboxComp;

    // Keep track of which notify CDOs we’ve bound so we can unbind safely
    UPROPERTY() TArray<TObjectPtr<UAnimNotifyState_MCSHitboxWindow>> BoundHitboxNotifies;

    // Subscribe/unsubscribe to all hitbox notifies used by a montage
    void BindHitboxNotifiesForMontage(UAnimMontage* Montage);
    void UnbindAllHitboxNotifies();

    // Callback targets for notify broadcasts
    UFUNCTION() void HandleHitboxNotifyBegin(FMCS_AttackHitbox& Hitbox);
    UFUNCTION() void HandleHitboxNotifyEnd(FMCS_AttackHitbox& Hitbox);
};

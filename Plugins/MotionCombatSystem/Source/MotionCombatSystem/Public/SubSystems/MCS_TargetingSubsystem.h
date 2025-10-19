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
 * MCS_TargetingSubsystem.h
 *
 * Description:
 *  UWorldSubsystem responsible for managing all valid combat targets in the current world.
 *  It is instantiated automatically for each active level/world (e.g., during level load).
 *  The subsystem scans the environment at set intervals, tracks valid enemies implementing
 *  the UMCS_CombatTargetInterface, and maintains an up-to-date list of nearby targets.
 *
 *  This subsystem exists per-world, not globally, and is recreated when a new level is loaded.
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include <Interfaces/MCS_CombatTargetInterface.h>
#include <Structs/MCS_TargetInfo.h>
#include "MCS_TargetingSubsystem.generated.h"

class AActor;


/*
 * Delegates
*/

// Delegate broadcast when the target list is updated
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTargetsUpdatedSignature, const TArray<FMCS_TargetInfo>&, NewTargetList, int32, NumTargets);


/**
 * UWorldSubsystem that manages all combat targets within the current world.
 */
UCLASS(BlueprintType, Blueprintable, meta=(DisplayName = "Motion Combat Targeting Subsystem"))
class MOTIONCOMBATSYSTEM_API UMCS_TargetingSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	// Constructor
	UMCS_TargetingSubsystem();

	/*
	 * Functions
	*/

	/** Register an actor as a valid combat target */
	UFUNCTION(BlueprintCallable, Category = "MCS|Targeting")
	void RegisterTarget(AActor* TargetActor);

	/** Unregister an actor from the target list */
	UFUNCTION(BlueprintCallable, Category = "MCS|Targeting")
	void UnregisterTarget(AActor* TargetActor);

	/** Returns a list of all current valid targets */
	UFUNCTION(BlueprintCallable, Category = "MCS|Targeting")
	const TArray<FMCS_TargetInfo>& GetAllTargets() const { return RegisteredTargets; }

	/** Finds the closest target to the given world position */
	UFUNCTION(BlueprintCallable, Category = "MCS|Targeting")
	AActor* GetClosestTarget(const FVector& FromLocation, float MaxRange = 2000.0f) const;

	/** Manually triggers a target scan (if you want to force-update) */
	UFUNCTION(BlueprintCallable, Category = "MCS|Targeting")
	void ScanForTargets();

	/** Enables or disables automatic target scanning */
	UFUNCTION(BlueprintCallable, Category = "MCS|Targeting")
	void SetTargetScanningEnabled(bool bEnable);

	/** Returns whether target scanning is currently active */
	UFUNCTION(BlueprintPure, Category = "MCS|Targeting")
	bool IsTargetScanningEnabled() const { return bIsScanningEnabled; }

	// =========================
	// WorldSubsystem lifecycle overrides
	// =========================

	// Only create this subsystem for real game worlds (PIE & Game), not the Editor preview world.
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Optional: log when (de)initialized for sanity checks.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/*
	 * Properties
	*/

	/** Event triggered whenever the RegisteredTargets array changes (added or removed). */
	UPROPERTY(BlueprintAssignable, Category = "Targeting|Events")
	FOnTargetsUpdatedSignature OnTargetsUpdated;

protected:
	/*
	 * Properties
	 */
	
	 /** How often to scan for targets (in seconds). Higher = less frequent. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Targeting|Performance")
	float TargetScanInterval = 1.0f;

	/** Maximum distance to detect potential targets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Targeting|Detection")
	float ScanRadius = 2500.0f;

	/** List of registered target info structs */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MCS|Targeting")
	TArray<FMCS_TargetInfo> RegisteredTargets;

	/** Whether to draw debug visuals for targeting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Targeting|Debug")
	bool bDebug = false;

	/*
	 * Functions
	*/

	/** Helper to validate and clean target list */
	void CleanupInvalidTargets();

private:
	/*
	 * Properties
	 */
	
	/** Cached reference to the world that owns this subsystem */
	TObjectPtr<UWorld> CachedWorld;

	/** Timer handle for recurring scans */
	FTimerHandle ScanTimerHandle;

	// Handy label we’ll use in logs so we can see which world is speaking.
	FString MakeWorldTag() const;

	// Last known target count, used to avoid redundant broadcasts
	int32 LastTargetCount = 0;

	/** Whether target scanning is currently active */
	bool bIsScanningEnabled = true;
	
	/*
	 * Functions
	*/

	/** Removes any targets that are valid but have moved beyond the current ScanRadius */
	void RemoveOutOfRangeTargets(const FVector& FromLocation);

};

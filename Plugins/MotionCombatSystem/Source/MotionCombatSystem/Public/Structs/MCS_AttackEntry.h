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
 * FMCS_AttackEntry.h
 * Declares the FMCS_AttackEntry struct used to describe an attack entry stored in
 * DataTables and referenced by the Motion Combat System (chooser & core).
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Animation/AnimMontage.h"
#include "UObject/ObjectMacros.h"
#include "GameplayTagContainer.h"
#include <Enums/EMCS_AttackTypes.h>
#include <Enums/EMCS_AttackDirections.h>
#include <Enums/EMCS_AttackSituations.h>
#include <Structs/MCS_AttackHitbox.h>
#include <Structs/MCS_AttackCondition.h>
#include "MCS_AttackEntry.generated.h"


/**
 * FMCS_AttackEntry
 * Simplified core structure used in Motion Combat System.
 * Supports DataTable integration and Blueprint editing.
 */
USTRUCT(BlueprintType, meta = (DisplayName = "Motion Combat System Attack Entry"))
struct MOTIONCOMBATSYSTEM_API FMCS_AttackEntry : public FTableRowBase
{
	GENERATED_BODY()

public:

	// Default constructor with sensible defaults
	FMCS_AttackEntry()
		: AttackName(NAME_None)
		, AttackType(EMCS_AttackType::Unknown)
		, AttackMontage(nullptr)
		, MontageSection(NAME_None)
		, Damage(0.f)
		, RangeStart(0.f)
		, RangeEnd(150.f)
		, SelectionWeight(1.f)
	{
		// Intentionally left blank
	}

	/*
	 * Properties
	*/

	/* ---------------------------
	 * Identification & categories
	 * --------------------------- */

	 // Optionally categorize this attack for organizational purposes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Attack", meta = (DisplayName = "Category"))
	FName Category = NAME_None;

	// Friendly name for designer display
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Attack", meta = (DisplayName = "Name"))
	FName AttackName;

	// Category/type used by chooser to group/select attacks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Attack", meta = (DisplayName = "Attack Type"))
	EMCS_AttackType AttackType = EMCS_AttackType::Unknown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Attack", meta = (DisplayName = "Situation"))
	EMCS_AttackSituations AttackSituation = EMCS_AttackSituations::Grounded;

	// Direction this attack is intended to be used from
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Attack", meta = (DisplayName = "Direction"))
	EMCS_AttackDirection AttackDirection = EMCS_AttackDirection::Forward;

	// Optional list of numeric conditions (e.g., Speed > 600)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Attack|Condition")
	TArray<FMCS_AttackCondition> ConditionalChecks;

	/* ---------------------------
	 * Animation / Montage
	 * --------------------------- */

	// The montage to play for this attack (TObjectPtr preferred over raw pointer)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Attack|Montage", meta = (DisplayName = "Montage"))
	TObjectPtr<UAnimMontage> AttackMontage = nullptr;

	// Optionally play from a specific montage section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Attack|Montage", meta = (DisplayName = "Montage Section"))
	FName MontageSection = NAME_None;

	/** Time to blend into this montage (seconds). Default: 0.10f */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Attack|Montage", meta = (ClampMin = "0.0", DisplayName = "Blend In Time"))
	float BlendInTime = 0.10f;

	/** Time to blend out of this montage (seconds). Default: 0.20f */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Attack|Montage", meta = (ClampMin = "0.0", DisplayName = "Blend Out Time"))
	float BlendOutTime = 0.20f;

	/* ---------------------------
	 * Gameplay values
	 * --------------------------- */

	// Primary damage this attack inflicts (designer-tunable)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Attack", meta = (ClampMin = "0.0", DisplayName = "Damage"))
	float Damage = 0.f;

	// Range start and end define the distance window for which this attack is valid
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Attack", meta = (ClampMin = "0.0", DisplayName = "Range Start"))
	float RangeStart = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Attack", meta = (ClampMin = "0.0", DisplayName = "Range End"))
	float RangeEnd = 150.f;

	/* ---------------------------
	 * Utility / Editor-only
	 * --------------------------- */

	// Designer weight to prefer certain attacks when multiple are valid (higher = more likely)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Attack", meta = (ClampMin = "0.0", DisplayName = "Selection Weight"))
	float SelectionWeight = 1.0f;

	/* ---------------------------
	 * Gameplay Tags (for GAS)
	 * --------------------------- */

	 // Primary gameplay tag used to identify or trigger this attack in GAS.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Attack", meta = (DisplayName = "Attack Gameplay Tag"))
	FGameplayTag AttackTag;

	/* ---------------------------
	 * Combo System
	 * --------------------------- */

	/** List of attack names that can follow this attack in a combo sequence. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Attack|Combo", meta = (DisplayName = "Allowed Next Attacks"))
	TArray<FName> AllowedNextAttacks;

	/*
	 * Functions
	*/

	/**
	 * Convenience helper:
	 * Returns true if the Montage reference is valid and has animation length > 0.
	 */
	FORCEINLINE bool HasValidMontage() const
	{
		return AttackMontage != nullptr && AttackMontage->GetPlayLength() > KINDA_SMALL_NUMBER;
	}

	/**
	 * Returns the play length of the montage in seconds, or 0.0f if invalid.
	 * Note: This is safe to call at runtime (returns 0 if Montage is null).
	 */
	FORCEINLINE float GetMontageLength() const
	{
		return AttackMontage ? AttackMontage->GetPlayLength() : 0.f;
	}

	/** Returns true if this attack's tag matches the provided tag. */
	FORCEINLINE bool MatchesTag(const FGameplayTag& TagToCheck) const
	{
		return AttackTag == TagToCheck;
	}

	// Checks if the given distance fits within the attack's valid window.
	FORCEINLINE bool IsWithinRange(float Distance) const
	{
		return Distance >= RangeStart && Distance <= RangeEnd;
	}

	/*
	 * Equality operator: compares all relevant fields for equality.
	 * Note: This does not compare pointers directly, but rather their values.
	*/
	bool operator==(const FMCS_AttackEntry& Other) const
	{
		return AttackName == Other.AttackName
			&& AttackMontage == Other.AttackMontage
			&& AttackTag == Other.AttackTag;
	}
};

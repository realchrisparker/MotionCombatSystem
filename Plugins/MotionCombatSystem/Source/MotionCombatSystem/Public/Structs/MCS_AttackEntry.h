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
#include "MCS_AttackEntry.generated.h"

/**
 * Attack type enum - categorize attacks (used by chooser logic to filter/select).
 */
UENUM(BlueprintType)
enum class EFMCS_AttackType : uint8
{
	Light      UMETA(DisplayName = "Light"),
	Heavy      UMETA(DisplayName = "Heavy"),
	Charge     UMETA(DisplayName = "Charge"),
	Thrown     UMETA(DisplayName = "Thrown"),
	Special    UMETA(DisplayName = "Special"),
	Unknown    UMETA(DisplayName = "Unknown")
};

/**
 * FMCS_AttackEntry
 * Simplified core structure used in Motion Combat System.
 * Supports DataTable integration and Blueprint editing.
 */
USTRUCT(BlueprintType)
struct MOTIONCOMBATSYSTEM_API FMCS_AttackEntry : public FTableRowBase
{
	GENERATED_BODY()

public:

	// Default constructor with sensible defaults
	FMCS_AttackEntry()
		: AttackName(NAME_None)
		, AttackType(EFMCS_AttackType::Unknown)
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

	// Friendly name for designer display
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Attack")
	FName AttackName;

	// Category/type used by chooser to group/select attacks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Attack")
	EFMCS_AttackType AttackType = EFMCS_AttackType::Unknown;

	/* ---------------------------
	 * Animation / Montage
	 * --------------------------- */

	// The montage to play for this attack (TObjectPtr preferred over raw pointer)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Attack")
	TObjectPtr<UAnimMontage> AttackMontage = nullptr;

	// Optionally play from a specific montage section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Attack")
	FName MontageSection = NAME_None;

	/* ---------------------------
	 * Gameplay values
	 * --------------------------- */

	// Primary damage this attack inflicts (designer-tunable)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Attack", meta = (ClampMin = "0.0"))
	float Damage = 0.f;

	// Range start and end define the distance window for which this attack is valid
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Attack", meta = (ClampMin = "0.0"))
	float RangeStart = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Attack", meta = (ClampMin = "0.0"))
	float RangeEnd = 150.f;

	/* ---------------------------
	 * Utility / Editor-only
	 * --------------------------- */

	// Designer weight to prefer certain attacks when multiple are valid (higher = more likely)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Attack", meta = (ClampMin = "0.0"))
	float SelectionWeight = 1.0f;

	/* ---------------------------
	 * Gameplay Tags (for GAS)
	 * --------------------------- */

	 // Primary gameplay tag used to identify or trigger this attack in GAS.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Attack|GAS")
	FGameplayTag AttackTag;

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

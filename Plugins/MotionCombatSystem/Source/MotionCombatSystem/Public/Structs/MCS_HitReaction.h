/*
 * ========================================================================
 * Copyright © 2025 God's Studio
 * All Rights Reserved.
 *
 * Free for all to use, copy, and distribute. I hope you learn from this as I learned creating it.
 * =============================================================================
 *
 * Project: Paragon GAS Sample
 * Author: Christopher D. Parker
 * Date: 9-6-2025
 * =============================================================================
 * MCS_HitReaction.h
 *
 * Defines hit reaction enums and struct for data-driven hit reactions.
 */

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimMontage.h"
#include "MCS_HitReaction.generated.h"


/*
* Enum defining Direction for character hit reactions.
*/
UENUM(BlueprintType, meta = (DisplayName = "Direction"))
enum class EMCS_Direction : uint8
{
    None        UMETA(DisplayName = "None"),
    Forward     UMETA(DisplayName = "Forward"),
    Back        UMETA(DisplayName = "Back"),
    Left        UMETA(DisplayName = "Left"),
    Right       UMETA(DisplayName = "Right")
};

/*
 * Enum defining Severity for character hit reactions.
*/
UENUM(BlueprintType, meta = (DisplayName = "Hit Severity"))
enum class EPGAS_HitSeverity : uint8
{
    Light       UMETA(DisplayName = "Light (Quick Flinch)"),
    Heavy       UMETA(DisplayName = "Heavy (Strong Hit)"),
    Stagger     UMETA(DisplayName = "Stagger (Medium Interrupt)"),
    Knockback   UMETA(DisplayName = "Knockback (Pushed Back)"),
    Knockdown   UMETA(DisplayName = "Knockdown (Fall to Ground)"),
    Dazed       UMETA(DisplayName = "Dazed (Stunned / Groggy)"),
    Critical    UMETA(DisplayName = "Critical (Launch / Fatal)"),
    Death       UMETA(DisplayName = "Death (Die / Ragdoll)"),
};


/**
 * Data-driven hit reaction struct.
 * Used instead of hard-coded montages for flexibility.
 */
USTRUCT(BlueprintType, Blueprintable, meta = (DisplayName = "Hit Reaction"))
struct MOTIONCOMBATSYSTEM_API FMCS_HitReaction
{
    GENERATED_BODY()

public:

    /** The direction this reaction is valid for. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction")
    EMCS_Direction Direction = EMCS_Direction::None;

    /** The severity of the hit (light, heavy, stagger, knockback, knockdown, dazed, critical). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction")
    EPGAS_HitSeverity Severity = EPGAS_HitSeverity::Light;

    /** The animation montage to play for this reaction. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction")
    TObjectPtr<UAnimMontage> Montage;

    /** Optional play rate for the montage. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction", meta = (ClampMin = "0.1", UIMin = "0.1"))
    float PlayRate = 1.0f;

    // ------------------------
    // Knockdown-specific
    // ------------------------

    /** Time to pause after knockdown while character is on the ground before get-up. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction|Knockdown", meta = (DisplayName = "On Ground Time", ClampMin = "0.0", UIMin = "0.0"))
    float OnGround = 1.0f;

    /** Montage to play when character gets up from knockdown. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction|Knockdown", meta = (DisplayName = "Get Up Montage"))
    TObjectPtr<UAnimMontage> GetUpMontage;

    // ------------------------
    // Dazed-specific
    // ------------------------

    /** Time character remains dazed. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction|Dazed", meta = (DisplayName = "Dazed Time"))
    float DazedTime = 1.0f;

    /** Montage to play when recovering from dazed state. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction|Dazed", meta = (DisplayName = "Recovery Montage"))
    TObjectPtr<UAnimMontage> RecoveryMontage;
};

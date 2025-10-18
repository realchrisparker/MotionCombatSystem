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
 * Date: 8-28-2025
 * =============================================================================
 * Simplified Hitbox Component (v1) – performs a basic socket-driven sphere trace.
 */

#include "Components/MCS_CombatHitboxComponent.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UMCS_CombatHitboxComponent::UMCS_CombatHitboxComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false; // tick only when detecting
}

void UMCS_CombatHitboxComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UMCS_CombatHitboxComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsDetecting)
    {
        PerformSweep();
    }
}

void UMCS_CombatHitboxComponent::StartHitDetection(const FMCS_AttackEntry& Attack, const FMCS_AttackHitbox& Hitbox)
{
    // ActiveHitbox = Attack.Hitbox; // cache hitbox from AttackType
    ActiveAttack = Attack;          // cache full attack type
    ActiveHitbox = Hitbox;          // cache hitbox
    bIsDetecting = true;

    AlreadyHitActors.Reset(); // clear at start of swing

    // Cache initial socket positions
    if (USkeletalMeshComponent* Mesh = ResolveMesh())
    {
        PrevStartLoc = Mesh->GetSocketLocation(ActiveHitbox.StartSocket);
        PrevEndLoc = Mesh->GetSocketLocation(ActiveHitbox.EndSocket);
    }

    SetComponentTickEnabled(true); // enable ticking
}

void UMCS_CombatHitboxComponent::StopHitDetection()
{
    bIsDetecting = false;
    AlreadyHitActors.Reset(); // clear at end of swing
    SetComponentTickEnabled(false); // disable ticking
}

void UMCS_CombatHitboxComponent::PerformSweep()
{
    // Get mesh component
    USkeletalMeshComponent* Mesh = ResolveMesh();

    // Validate mesh and sockets
    if (!Mesh || ActiveHitbox.StartSocket == NAME_None || ActiveHitbox.EndSocket == NAME_None)
        return;

    // Get current socket locations
    const FVector CurrStart = Mesh->GetSocketLocation(ActiveHitbox.StartSocket);
    const FVector CurrEnd = Mesh->GetSocketLocation(ActiveHitbox.EndSocket);

    // Sweep multiple times between previous and current positions (substepping)
    for (int32 i = 0; i < SubstepCount; i++)
    {
        const float Alpha = (i + 1) / static_cast<float>(SubstepCount);

        const FVector StepStart = FMath::Lerp(PrevStartLoc, CurrStart, Alpha);
        const FVector StepEnd = FMath::Lerp(PrevEndLoc, CurrEnd, Alpha);

        FCollisionQueryParams Params(SCENE_QUERY_STAT(PGAS_Hitbox), false, GetOwner()); // Setup collision parameters
        TArray<FHitResult> Hits; // Array to store hit results

        // Perform the sweep
        GetWorld()->SweepMultiByObjectType(
            Hits,
            StepStart,
            StepEnd,
            FQuat::Identity,
            FCollisionObjectQueryParams(ECC_Pawn),
            FCollisionShape::MakeSphere(ActiveHitbox.Radius),
            Params
        );

        // Process hit results
        for (const FHitResult& Hit : Hits)
        {
            if (AActor* HitActor = Hit.GetActor())
            {
                if (HitActor == GetOwner()) // skip self
                    continue;

                if (AlreadyHitActors.Contains(HitActor)) // skip duplicate hits in same swing
                    continue;

                AlreadyHitActors.Add(HitActor); // mark as hit
                OnHitboxHit.Broadcast(HitActor, Hit, ActiveAttack); // Broadcast hit event

                if (ActiveHitbox.bDebugDraw)
                {
                    DrawDebugSphere(GetWorld(), Hit.ImpactPoint, ActiveHitbox.Radius, 12, FColor::Red, false, 0.05f);
                }
            }
        }

        // Draw sweep line
        if (ActiveHitbox.bDebugDraw)
        {
            DrawDebugLine(GetWorld(), StepStart, StepEnd, FColor::Green, false, 0.05f, 0, 1.5f);
        }
    }

    // Update previous socket locations for next frame
    PrevStartLoc = CurrStart;
    PrevEndLoc = CurrEnd;

    // Draw socket spheres
    if (ActiveHitbox.bDebugDraw)
    {
        DrawDebugSphere(GetWorld(), CurrStart, ActiveHitbox.Radius, 8, FColor::Blue, false, 0.05f);
        DrawDebugSphere(GetWorld(), CurrEnd, ActiveHitbox.Radius, 8, FColor::Blue, false, 0.05f);
    }
}

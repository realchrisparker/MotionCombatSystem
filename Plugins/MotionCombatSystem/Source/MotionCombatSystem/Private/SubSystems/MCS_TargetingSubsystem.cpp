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
 * MCS_TargetingSubsystem.cpp
 * Implementation for the world subsystem responsible for managing active combat targets.
 */

#include <SubSystems/MCS_TargetingSubsystem.h>
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "CollisionQueryParams.h"
#include "CollisionShape.h"
#include "DrawDebugHelpers.h"

UMCS_TargetingSubsystem::UMCS_TargetingSubsystem()
{
}

bool UMCS_TargetingSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    // Only create for PIE/Game worlds; ignore Editor worlds that cause duplicate ticking/default values.
    const UWorld* World = Cast<UWorld>(Outer);
    return (World && World->IsGameWorld());
}

void UMCS_TargetingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    CachedWorld = GetWorld();
    if (!IsValid(CachedWorld))
    {
        UE_LOG(LogTemp, Warning, TEXT("[MCS_TargetingSubsystem] Failed to cache valid world on Initialize."));
        return;
    }

    // Start recurring scan timer
    CachedWorld->GetTimerManager().SetTimerForNextTick([ this ] ()
        {
            if (IsValid(CachedWorld))
            {
                CachedWorld->GetTimerManager().SetTimer(
                    ScanTimerHandle,
                    this,
                    &UMCS_TargetingSubsystem::ScanForTargets,
                    TargetScanInterval,
                    true);
            }
        });

    // UE_LOG(LogTemp, Log, TEXT("%s UMCS_TargetingSubsystem::Initialize (ScanRadius=%.0f)"),
    //     *MakeWorldTag(), ScanRadius);
    UE_LOG(LogTemp, Log, TEXT("[MCS_TargetingSubsystem] Initialized in World: %s, Type: %d"),
        *CachedWorld->GetName(), (int32)CachedWorld->WorldType);
}

void UMCS_TargetingSubsystem::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("%s UMCS_TargetingSubsystem::Deinitialize"), *MakeWorldTag());
    CachedWorld = nullptr;
    Super::Deinitialize();
}

FString UMCS_TargetingSubsystem::MakeWorldTag() const
{
    const UWorld* World = GetWorld();
    if (!World) return TEXT("[World:?]");

    // Build a readable tag like: [World: PIE_1, NM_Client]
    FString WorldName = World->GetName();
#if WITH_EDITOR
    // Distinguish PIE instances
    if (World->WorldType == EWorldType::PIE)
    {
        WorldName = TEXT("PIE_World");
    }
#endif

    const ENetMode NM = World->GetNetMode();
    const TCHAR* NetModeStr =
        (NM == NM_DedicatedServer) ? TEXT("NM_DedicatedServer") :
        (NM == NM_ListenServer) ? TEXT("NM_ListenServer") :
        (NM == NM_Client) ? TEXT("NM_Client") :
        TEXT("NM_Standalone");

    return FString::Printf(TEXT("[World: %s, %s]"), *WorldName, NetModeStr);
}

void UMCS_TargetingSubsystem::RegisterTarget(AActor* TargetActor)
{
    if (!TargetActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("UMCS_TargetingSubsystem::RegisterTarget - Invalid actor."));
        return;
    }

    for (const FMCS_TargetInfo& Info : RegisteredTargets)
    {
        if (Info.TargetActor == TargetActor)
            return;
    }

    FMCS_TargetInfo NewTarget;
    NewTarget.TargetActor = TargetActor;
    NewTarget.bIsValid = true;

    RegisteredTargets.Add(NewTarget);

    UE_LOG(LogTemp, Warning, TEXT("[MCS_TargetingSubsystem] Registered Target: %s"), *TargetActor->GetName());
}

void UMCS_TargetingSubsystem::UnregisterTarget(AActor* TargetActor)
{
    if (!TargetActor)
        return;

    int32 Removed = RegisteredTargets.RemoveAll([ TargetActor ] (const FMCS_TargetInfo& Info)
        {
            return Info.TargetActor == TargetActor;
        });

    if (Removed > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[MCS_TargetingSubsystem] Unregistered Target: %s"), *TargetActor->GetName());
    }
}

void UMCS_TargetingSubsystem::CleanupInvalidTargets()
{
    RegisteredTargets.RemoveAll([ ] (const FMCS_TargetInfo& Info)
        {
            return !IsValid(Info.TargetActor);
        });
}

void UMCS_TargetingSubsystem::ScanForTargets()
{
    UWorld* World = CachedWorld.Get();
    if (!World) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!IsValid(PlayerPawn))
        return;

    const FVector PlayerLocation = PlayerPawn->GetActorLocation();

    // Clean up dead targets AND those now out of range
    CleanupInvalidTargets();
    RemoveOutOfRangeTargets(PlayerLocation);

    // Use a sphere overlap instead of scanning all actors in the world. Much more efficient.
    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MCS_TargetScan), false);
    FCollisionObjectQueryParams ObjectQueryParams(FCollisionObjectQueryParams::AllDynamicObjects);

    const FCollisionShape SphereShape = FCollisionShape::MakeSphere(ScanRadius);

    World->OverlapMultiByObjectType(
        Overlaps,
        PlayerLocation,
        FQuat::Identity,
        ObjectQueryParams,
        SphereShape,
        QueryParams);

    // Visualization for debugging if enabled
    if (bDebugDrawTargeting)
    {
        DrawDebugSphere(World, PlayerLocation, ScanRadius, 16, FColor::Red, false, 0.25f);
    }

    // Convert results to actor list
    TArray<AActor*> FoundActors;
    FoundActors.Reserve(Overlaps.Num());
    for (const FOverlapResult& Result : Overlaps)
    {
        if (AActor* HitActor = Result.GetActor())
        {
            FoundActors.Add(HitActor);
        }
    }

    // Process each found actor to determine if we should add them to our target list
    for (AActor* Actor : FoundActors)
    {
        if (!IsValid(Actor) || Actor->IsActorBeingDestroyed())
            continue;

        // Ignore player pawn
        if (Actor == UGameplayStatics::GetPlayerPawn(World, 0))
            continue;

        // Must implement the MCS combat target interface
        if (!Actor->Implements<UMCS_CombatTargetInterface>())
            continue;

        // Ask the actor if it can currently be targeted
        if (!IMCS_CombatTargetInterface::Execute_CanBeTargeted(Actor))
            continue;

        const float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
        if (Distance > ScanRadius)
            continue;

        // Avoid duplicates
        bool bAlreadyRegistered = false;
        for (const FMCS_TargetInfo& Info : RegisteredTargets)
        {
            if (Info.TargetActor == Actor)
            {
                bAlreadyRegistered = true;
                break;
            }
        }

        if (!bAlreadyRegistered)
        {
            FMCS_TargetInfo NewTarget;
            NewTarget.TargetActor = Actor;
            NewTarget.DistanceFromPlayer = Distance;
            NewTarget.bIsValid = true;
            RegisteredTargets.Add(NewTarget);

            UE_LOG(LogTemp, Warning, TEXT("[MCS_TargetingSubsystem] Added Target: %s"), *Actor->GetName());
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("[MCS_TargetingSubsystem] Scanned %d valid targets within %.0f units."), RegisteredTargets.Num(), ScanRadius);
}

AActor* UMCS_TargetingSubsystem::GetClosestTarget(const FVector& FromLocation, float MaxRange) const
{
    AActor* ClosestActor = nullptr;
    float ClosestDistanceSq = MaxRange * MaxRange;

    for (const FMCS_TargetInfo& Info : RegisteredTargets)
    {
        if (!IsValid(Info.TargetActor))
            continue;

        const float DistSq = FVector::DistSquared(FromLocation, Info.TargetActor->GetActorLocation());
        if (DistSq < ClosestDistanceSq)
        {
            ClosestDistanceSq = DistSq;
            ClosestActor = Info.TargetActor;
        }
    }

    return ClosestActor;
}

void UMCS_TargetingSubsystem::RemoveOutOfRangeTargets(const FVector& FromLocation)
{
    if (RegisteredTargets.Num() == 0)
        return;

    RegisteredTargets.RemoveAll([ this, &FromLocation ] (const FMCS_TargetInfo& Info)
        {
            if (!IsValid(Info.TargetActor))
                return true; // Remove invalids

            const float Distance = FVector::Dist(FromLocation, Info.TargetActor->GetActorLocation());
            return Distance > ScanRadius; // Remove if too far
        });
}
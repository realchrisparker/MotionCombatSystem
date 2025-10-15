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
 * MCS_CombatCoreComponent.cpp
 * Implements the logic for loading, choosing, and executing attacks
 * using the targeting subsystem and data tables.
 */

#include <Components/MCS_CombatCoreComponent.h>
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"


 // Constructor
UMCS_CombatCoreComponent::UMCS_CombatCoreComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UMCS_CombatCoreComponent::BeginPlay()
{
    Super::BeginPlay();

    // Get the targeting subsystem from world
    if (UWorld* World = GetWorld())
    {
        TargetingSubsystem = World->GetSubsystem<UMCS_TargetingSubsystem>();
    }

    // Ensure chooser exists
    if (!AttackChooser)
    {
        AttackChooser = NewObject<UMCS_AttackChooser>(this, UMCS_AttackChooser::StaticClass());
    }

    // Load attacks from DataTable if assigned
    LoadAttacksFromDataTable();
}

/*
 * Loads all FMCS_AttackEntry rows from the DataTable into the AttackChooser
 */
void UMCS_CombatCoreComponent::LoadAttacksFromDataTable()
{
    if (!AttackDataTable)
    {
        if (bDebugCombat)
            UE_LOG(LogTemp, Warning, TEXT("[CombatCore] No AttackDataTable assigned."));
        return;
    }

    TArray<FMCS_AttackEntry*> RowPtrs;
    AttackDataTable->GetAllRows(TEXT("LoadAttacksFromDataTable"), RowPtrs);

    if (AttackChooser)
    {
        AttackChooser->AttackEntries.Reset();
        for (const FMCS_AttackEntry* Row : RowPtrs)
        {
            if (Row)
                AttackChooser->AttackEntries.Add(*Row);
        }

        if (bDebugCombat)
            UE_LOG(LogTemp, Warning, TEXT("[CombatCore] Loaded %d attacks from DataTable."), AttackChooser->AttackEntries.Num());
    }
}

/*
 * Chooses an appropriate attack using AttackChooser and available targets
 */
bool UMCS_CombatCoreComponent::SelectAttack(EFMCS_AttackType DesiredType)
{
    if (!AttackChooser)
        return false;

    AActor* OwnerActor = GetOwnerActor();
    if (!OwnerActor)
        return false;

    // Filter attacks by requested type
    TArray<FMCS_AttackEntry> FilteredEntries;
    for (const FMCS_AttackEntry& Entry : AttackChooser->AttackEntries)
    {
        if (Entry.AttackType == DesiredType)
        {
            FilteredEntries.Add(Entry);
        }
    }

    if (FilteredEntries.Num() == 0)
    {
        if (bDebugCombat)
            UE_LOG(LogTemp, Warning, TEXT("[CombatCore] No attacks found of type: %d"), (uint8)DesiredType);
        return false;
    }

    // Get targets from subsystem
    TArray<AActor*> Targets;
    if (TargetingSubsystem)
    {
        for (const FMCS_TargetInfo& Info : TargetingSubsystem->GetAllTargets())
        {
            if (IsValid(Info.TargetActor))
                Targets.Add(Info.TargetActor);
        }
    }

    // Create a temporary chooser clone (optional) or just reuse
    FMCS_AttackEntry ChosenAttack;

    // Temporarily use FilteredEntries instead of full list
    TArray<FMCS_AttackEntry> OriginalEntries = AttackChooser->AttackEntries;
    AttackChooser->AttackEntries = FilteredEntries;

    const bool bSuccess = AttackChooser->ChooseAttack(OwnerActor, Targets, ChosenAttack);

    // Restore full list after selection
    AttackChooser->AttackEntries = OriginalEntries;

    if (bSuccess)
    {
        CurrentAttack = ChosenAttack;

        if (bDebugCombat)
        {
            UE_LOG(LogTemp, Log, TEXT("[CombatCore] Selected %s attack: %s"),
                *UEnum::GetValueAsString(DesiredType),
                *CurrentAttack.AttackName.ToString());
        }
    }

    return bSuccess;
}

/*
 * Plays the selected attack's montage if valid
 */
void UMCS_CombatCoreComponent::PerformAttack(EFMCS_AttackType DesiredType)
{
    if (SelectAttack(DesiredType))
    {
        ACharacter* CharacterOwner = Cast<ACharacter>(GetOwner());
        if (!CharacterOwner || !CurrentAttack.HasValidMontage())
        {
            if (bDebugCombat)
                UE_LOG(LogTemp, Warning, TEXT("[CombatCore] Invalid attack or character."));
            return;
        }

        UAnimInstance* AnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();
        if (AnimInstance && CurrentAttack.AttackMontage)
        {
            AnimInstance->Montage_Play(CurrentAttack.AttackMontage);
            if (CurrentAttack.MontageSection != NAME_None)
            {
                AnimInstance->Montage_JumpToSection(CurrentAttack.MontageSection, CurrentAttack.AttackMontage);
            }

            if (bDebugCombat)
                UE_LOG(LogTemp, Log, TEXT("[CombatCore] Playing montage: %s"), *CurrentAttack.AttackMontage->GetName());
        }
    }
    else if (bDebugCombat)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CombatCore] PerformAttack failed - no valid %s attack found."),
            *UEnum::GetValueAsString(DesiredType));
    }
}

/*
 * Gets the closest valid target via TargetingSubsystem
 */
AActor* UMCS_CombatCoreComponent::GetClosestTarget(float MaxRange) const
{
    if (!TargetingSubsystem)
        return nullptr;

    if (AActor* OwnerActor = GetOwnerActor())
    {
        return TargetingSubsystem->GetClosestTarget(OwnerActor->GetActorLocation(), MaxRange);
    }

    return nullptr;
}

/*
 * Utility to get the owning actor safely
 */
AActor* UMCS_CombatCoreComponent::GetOwnerActor() const
{
    return GetOwner();
}

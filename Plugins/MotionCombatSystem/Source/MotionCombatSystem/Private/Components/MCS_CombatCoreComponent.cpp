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

    if (GetOwnerActor())
    {
        // Cache hitbox component reference
        CachedHitboxComp = GetOwnerActor()->FindComponentByClass<UMCS_CombatHitboxComponent>();
    }

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

    // Bind to targeting updates
    if (TargetingSubsystem)
    {
        TargetingSubsystem->OnTargetsUpdated.AddDynamic(this, &UMCS_CombatCoreComponent::HandleTargetsUpdated);
    }

}

// Loads all FMCS_AttackEntry rows from the DataTable into the AttackChooser
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
 * @param DesiredType - type of attack to select
 * @param DesiredDirection - direction of the attack in world space
*/
bool UMCS_CombatCoreComponent::SelectAttack(EMCS_AttackType DesiredType, EMCS_AttackDirection DesiredDirection)
{
    if (!AttackChooser)
        return false;

    AActor* OwnerActor = GetOwnerActor();
    if (!OwnerActor)
    {
        return false;
    }

    // Filter attacks by requested type
    TArray<FMCS_AttackEntry> FilteredEntries;
    for (const FMCS_AttackEntry& Entry : AttackChooser->AttackEntries)
    {
        if (Entry.AttackType == DesiredType)
        {
            FilteredEntries.Add(Entry);
        }
    }

    if (FilteredEntries.IsEmpty())
    {
        if (bDebugCombat)
        {
            UE_LOG(LogTemp, Warning, TEXT("[CombatCore] No attacks found of type: %s"), *UEnum::GetValueAsString(DesiredType));
        }
        return false;
    }

    // Gather valid targets from the subsystem
    TArray<AActor*> Targets;
    if (TargetingSubsystem)
    {
        for (const FMCS_TargetInfo& Info : TargetingSubsystem->GetAllTargets())
        {
            if (IsValid(Info.TargetActor))
            {
                Targets.Add(Info.TargetActor);
            }
        }
    }

    // Use the filtered list temporarily
    FMCS_AttackEntry ChosenAttack;
    const TArray<FMCS_AttackEntry> OriginalEntries = AttackChooser->AttackEntries;
    AttackChooser->AttackEntries = FilteredEntries;

    const bool bSuccess = AttackChooser->ChooseAttack(OwnerActor, Targets, DesiredDirection, ChosenAttack);

    // Restore full list
    AttackChooser->AttackEntries = OriginalEntries;

    if (bSuccess)
    {
        CurrentAttack = ChosenAttack;

        if (bDebugCombat)
        {
            UE_LOG(LogTemp, Log, TEXT("[CombatCore] Selected %s attack: %s (Direction: %s)"), *UEnum::GetValueAsString(DesiredType), *CurrentAttack.AttackName.ToString(), *UEnum::GetValueAsString(CurrentAttack.AttackDirection));
        }
    }

    return bSuccess;
}

/*
 * Plays the selected attack's montage if valid
 * @param DesiredType - type of attack to perform
 * @param DesiredDirection - direction of the attack in world space
*/
void UMCS_CombatCoreComponent::PerformAttack(EMCS_AttackType DesiredType, EMCS_AttackDirection DesiredDirection)
{
    if (!SelectAttack(DesiredType, DesiredDirection))
    {
        if (bDebugCombat)
        {
            UE_LOG(LogTemp, Warning, TEXT("[CombatCore] PerformAttack failed — no valid %s attack for %s."),
                *UEnum::GetValueAsString(DesiredType), *UEnum::GetValueAsString(DesiredDirection));
        }
        return;
    }

    ACharacter* CharacterOwner = Cast<ACharacter>(GetOwner());
    if (!CharacterOwner || !CurrentAttack.HasValidMontage()) { return; }

    // Cache the hitbox component
    CachedHitboxComp = CharacterOwner->FindComponentByClass<UMCS_CombatHitboxComponent>();

    // Bind to the notify broadcasts for THIS montage
    BindHitboxNotifiesForMontage(CurrentAttack.AttackMontage);

    UAnimInstance* AnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();
    if (!AnimInstance) return;

    // Optional: stop any previous montage to avoid overlap
    AnimInstance->Montage_Stop(0.f);

    // Play montage
    AnimInstance->Montage_Play(CurrentAttack.AttackMontage);
    if (CurrentAttack.MontageSection != NAME_None)
    {
        AnimInstance->Montage_JumpToSection(CurrentAttack.MontageSection, CurrentAttack.AttackMontage);
    }

    if (bDebugCombat)
    {
        UE_LOG(LogTemp, Log, TEXT("[CombatCore] Playing montage: %s"), *CurrentAttack.AttackMontage->GetName());
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

// Handler for TargetingSubsystem target updates
void UMCS_CombatCoreComponent::HandleTargetsUpdated(const TArray<FMCS_TargetInfo>& NewTargets, int32 NewTargetCount)
{
    if (bDebugCombat)
    {
        UE_LOG(LogTemp, Log, TEXT("[CombatCore] Target list changed: %d targets in range."), NewTargetCount);
    }

    // Fire the exposed Blueprint event
    if (OnTargetingUpdated.IsBound())
    {
        OnTargetingUpdated.Broadcast(NewTargets, NewTargetCount);
    }
}

/**
 * Utility to convert 2D movement input into an EMCS_AttackDirection enum value
 * @param MoveInput - 2D movement input vector (X=Forward/Backward, Y=Left/Right)
 * @return Corresponding EMCS_AttackDirection value
 */
EMCS_AttackDirection UMCS_CombatCoreComponent::GetAttackDirection(const FVector2D& MoveInput) const
{
    // If no significant input, treat as Omni (neutral)
    if (MoveInput.IsNearlyZero(0.2f))
    {
        return EMCS_AttackDirection::Omni;
    }

    // Get the controlling actor (usually the player character)
    const AActor* OwnerActor = GetOwner();
    if (!OwnerActor)
    {
        return EMCS_AttackDirection::Omni;
    }

    // Get control rotation (camera-facing)
    FRotator ControlRot = OwnerActor->GetInstigatorController()
        ? OwnerActor->GetInstigatorController()->GetControlRotation()
        : FRotator::ZeroRotator;

    // Zero out pitch/roll — we only care about yaw
    ControlRot.Pitch = 0.f;
    ControlRot.Roll = 0.f;

    const FVector CameraForward = FRotationMatrix(ControlRot).GetUnitAxis(EAxis::X);
    const FVector CameraRight = FRotationMatrix(ControlRot).GetUnitAxis(EAxis::Y);

    // Convert 2D input into a world-space direction
    FVector DesiredDirectionWS = (CameraForward * MoveInput.Y + CameraRight * MoveInput.X).GetSafeNormal();

    if (DesiredDirectionWS.IsNearlyZero())
    {
        return EMCS_AttackDirection::Omni;
    }

    // Get forward vector of actor (used to compare relative direction)
    const FVector ActorForward = OwnerActor->GetActorForwardVector();
    const FVector ActorRight = OwnerActor->GetActorRightVector();

    // Calculate dot products
    const float ForwardDot = FVector::DotProduct(ActorForward, DesiredDirectionWS);
    const float RightDot = FVector::DotProduct(ActorRight, DesiredDirectionWS);

    // Determine facing quadrant using dot thresholds
    const float ForwardThreshold = 0.5f;  // cosine ~60 degrees
    const float SideThreshold = 0.5f;

    if (ForwardDot > ForwardThreshold)
    {
        return EMCS_AttackDirection::Forward;
    }
    else if (ForwardDot < -ForwardThreshold)
    {
        return EMCS_AttackDirection::Backward;
    }
    else if (RightDot > SideThreshold)
    {
        return EMCS_AttackDirection::Right;
    }
    else if (RightDot < -SideThreshold)
    {
        return EMCS_AttackDirection::Left;
    }

    // Fallback if between zones
    return EMCS_AttackDirection::Omni;
}

void UMCS_CombatCoreComponent::BindHitboxNotifiesForMontage(UAnimMontage* Montage)
{
    UnbindAllHitboxNotifies();
    if (!Montage) { return; }

    for (const FAnimNotifyEvent& Event : Montage->Notifies)
    {
        if (Event.NotifyStateClass && Event.NotifyStateClass->GetClass()->IsChildOf(UAnimNotifyState_MCSHitboxWindow::StaticClass()))
        {
            if (UAnimNotifyState_MCSHitboxWindow* Notify =
                Cast<UAnimNotifyState_MCSHitboxWindow>(Event.NotifyStateClass))
            {
                // Avoid duplicate binds
                Notify->OnNotifyBegin.RemoveDynamic(this, &UMCS_CombatCoreComponent::HandleHitboxNotifyBegin);
                Notify->OnNotifyEnd.RemoveDynamic(this, &UMCS_CombatCoreComponent::HandleHitboxNotifyEnd);

                Notify->OnNotifyBegin.AddDynamic(this, &UMCS_CombatCoreComponent::HandleHitboxNotifyBegin);
                Notify->OnNotifyEnd.AddDynamic(this, &UMCS_CombatCoreComponent::HandleHitboxNotifyEnd);

                BoundHitboxNotifies.Add(Notify);
            }
        }
    }

    if (bDebugCombat)
    {
        UE_LOG(LogTemp, Log, TEXT("[CombatCore] Bound to %d hitbox notifies on montage %s"),
            BoundHitboxNotifies.Num(), *Montage->GetName());
    }
}

void UMCS_CombatCoreComponent::UnbindAllHitboxNotifies()
{
    if (BoundHitboxNotifies.Num() == 0) { return; }

    for (UAnimNotifyState_MCSHitboxWindow* NotifyCDO : BoundHitboxNotifies)
    {
        if (NotifyCDO)
        {
            NotifyCDO->OnNotifyBegin.RemoveDynamic(this, &UMCS_CombatCoreComponent::HandleHitboxNotifyBegin);
            NotifyCDO->OnNotifyEnd.RemoveDynamic(this, &UMCS_CombatCoreComponent::HandleHitboxNotifyEnd);
        }
    }
    BoundHitboxNotifies.Reset();
}

void UMCS_CombatCoreComponent::HandleHitboxNotifyBegin(FMCS_AttackHitbox& Hitbox)
{
    // 🛡️ Guard: only run if this character is actively playing this montage
    if (const ACharacter* C = Cast<ACharacter>(GetOwner());
        !(C && C->GetMesh() && C->GetMesh()->GetAnimInstance() &&
            C->GetMesh()->GetAnimInstance()->Montage_IsPlaying(CurrentAttack.AttackMontage)))
        return;

    if (!CachedHitboxComp)
    {
        if (ACharacter* CharacterOwner = Cast<ACharacter>(GetOwner()))
        {
            CachedHitboxComp = CharacterOwner->FindComponentByClass<UMCS_CombatHitboxComponent>();
        }
    }
    if (!CachedHitboxComp) return;

    // ✅ Automatically reset hit tracking whenever a new hitbox window begins
    CachedHitboxComp->ResetAlreadyHit();

    // Start hit detection for this hitbox
    CachedHitboxComp->StartHitDetection(CurrentAttack, Hitbox);

    if (bDebugCombat)
    {
        UE_LOG(LogTemp, Log, TEXT("[CombatCore] Hitbox BEGIN (Start:%s End:%s R:%.1f)"), *Hitbox.StartSocket.ToString(), *Hitbox.EndSocket.ToString(), Hitbox.Radius);
    }
}

void UMCS_CombatCoreComponent::HandleHitboxNotifyEnd(FMCS_AttackHitbox& Hitbox)
{
    if (const ACharacter* C = Cast<ACharacter>(GetOwner());
        !(C && C->GetMesh() && C->GetMesh()->GetAnimInstance() &&
            C->GetMesh()->GetAnimInstance()->Montage_IsPlaying(CurrentAttack.AttackMontage)))
    {
        return;
    }

    if (CachedHitboxComp)
    {
        CachedHitboxComp->StopHitDetection();

        if (bDebugCombat)
        {
            UE_LOG(LogTemp, Log, TEXT("[CombatCore] Hitbox END (Label:%s)"),
                *Hitbox.StartSocket.ToString());
        }
    }
}
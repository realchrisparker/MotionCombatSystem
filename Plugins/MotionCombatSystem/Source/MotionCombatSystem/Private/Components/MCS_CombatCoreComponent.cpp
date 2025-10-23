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

    // // Ensure chooser exists
    // if (!AttackChooser)
    // {
    //     AttackChooser = NewObject<UMCS_AttackChooser>(this, UMCS_AttackChooser::StaticClass());
    // }

    // If no active set defined but map has entries, activate the first
    if (!ActiveAttackSetTag.IsValid() && AttackSets.Num() > 0)
    {
        const FGameplayTag FirstKey = AttackSets.CreateConstIterator()->Key;
        SetActiveAttackSet(FirstKey);
    }

    // Bind to targeting updates
    if (TargetingSubsystem)
    {
        TargetingSubsystem->OnTargetsUpdated.AddDynamic(this, &UMCS_CombatCoreComponent::HandleTargetsUpdated);
    }

}

/*
 * Chooses an appropriate attack using AttackChooser and available targets
 * @param DesiredType - type of attack to select
 * @param DesiredDirection - direction of the attack in world space
*/
bool UMCS_CombatCoreComponent::SelectAttack(EMCS_AttackType DesiredType, EMCS_AttackDirection DesiredDirection, const FMCS_AttackSituation& CurrentSituation)
{
    const FMCS_AttackSetData* ActiveSet = AttackSets.Find(ActiveAttackSetTag);
    if (!ActiveSet || !ActiveSet->AttackChooser)
    {
        UE_LOG(LogTemp, Error, TEXT("[CombatCore] No valid attack set or chooser for active tag: %s"), *ActiveAttackSetTag.ToString());
        return false;
    }

    UMCS_AttackChooser* Chooser = ActiveSet->AttackChooser;
    AActor* OwnerActor = GetOwnerActor();
    if (!OwnerActor) return false;

    // Filter by type
    TArray<FMCS_AttackEntry> FilteredEntries;
    for (const FMCS_AttackEntry& Entry : Chooser->AttackEntries)
    {
        if (Entry.AttackType == DesiredType)
        {
            FilteredEntries.Add(Entry);
        }
    }

    if (FilteredEntries.IsEmpty())
    {
        if (bDebug)
        {
            UE_LOG(LogTemp, Warning, TEXT("[CombatCore] No attacks found of type %s for set %s."), *UEnum::GetValueAsString(DesiredType), *ActiveAttackSetTag.ToString());
        }
        return false;
    }

    // Gather targets
    TArray<AActor*> Targets;
    if (TargetingSubsystem)
    {
        for (const FMCS_TargetInfo& Info : TargetingSubsystem->GetAllTargets())
            if (IsValid(Info.TargetActor))
                Targets.Add(Info.TargetActor);
    }

    FMCS_AttackEntry ChosenAttack;
    const TArray<FMCS_AttackEntry> OriginalEntries = Chooser->AttackEntries;
    Chooser->AttackEntries = FilteredEntries;

    const bool bSuccess = Chooser->ChooseAttack(OwnerActor, Targets, DesiredDirection, CurrentSituation, ChosenAttack);
    Chooser->AttackEntries = OriginalEntries;

    if (bSuccess)
    {
        CurrentAttack = ChosenAttack;

        if (bDebug)
        {
            UE_LOG(LogTemp, Log, TEXT("[CombatCore] Selected %s attack: %s (Dir: %s, Set: %s)"),
                *UEnum::GetValueAsString(DesiredType),
                *CurrentAttack.AttackName.ToString(),
                *UEnum::GetValueAsString(CurrentAttack.AttackDirection),
                *ActiveAttackSetTag.ToString());
        }
    }

    return bSuccess;
}

/*
 * Plays the selected attack's montage if valid
 * @param DesiredType - type of attack to perform
 * @param DesiredDirection - direction of the attack in world space
*/
void UMCS_CombatCoreComponent::PerformAttack(EMCS_AttackType DesiredType, EMCS_AttackDirection DesiredDirection, const FMCS_AttackSituation& CurrentSituation)
{
    if (!SelectAttack(DesiredType, DesiredDirection, CurrentSituation))
    {
        if (bDebug)
        {
            UE_LOG(LogTemp, Warning, TEXT("[CombatCore] PerformAttack failed — no valid attack found."));
        }
        return;
    }

    ACharacter* CharacterOwner = Cast<ACharacter>(GetOwner());
    if (!CharacterOwner || !CurrentAttack.HasValidMontage()) return;

    CachedHitboxComp = CharacterOwner->FindComponentByClass<UMCS_CombatHitboxComponent>();
    BindHitboxNotifiesForMontage(CurrentAttack.AttackMontage);

    UAnimInstance* AnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();
    if (!AnimInstance) return;

    AnimInstance->Montage_Stop(0.f);
    AnimInstance->Montage_Play(CurrentAttack.AttackMontage);
    if (CurrentAttack.MontageSection != NAME_None)
    {
        AnimInstance->Montage_JumpToSection(CurrentAttack.MontageSection, CurrentAttack.AttackMontage);
    }

    if (bDebug)
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
    if (bDebug)
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

    if (bDebug)
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

    if (bDebug)
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

        if (bDebug)
        {
            UE_LOG(LogTemp, Log, TEXT("[CombatCore] Hitbox END (Label:%s)"),
                *Hitbox.StartSocket.ToString());
        }
    }
}

/**
 * Sets the active attack DataTable using a gameplay tag.
 */
bool UMCS_CombatCoreComponent::SetActiveAttackSet(const FGameplayTag& NewAttackSetTag)
{
    const FMCS_AttackSetData* FoundSet = AttackSets.Find(NewAttackSetTag);
    if (!FoundSet)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CombatCore] Unknown attack set tag: %s"), *NewAttackSetTag.ToString());
        return false;
    }

    if (!FoundSet->AttackDataTable || !FoundSet->AttackChooser)
    {
        UE_LOG(LogTemp, Error, TEXT("[CombatCore] Attack set '%s' missing required DataTable or Chooser!"), *NewAttackSetTag.ToString());
        return false;
    }

    ActiveAttackSetTag = NewAttackSetTag;
    AttackDataTable = FoundSet->AttackDataTable;

    // Load data into that set’s Chooser
    TArray<FMCS_AttackEntry*> Rows;
    AttackDataTable->GetAllRows(TEXT("LoadFromSet"), Rows);

    FoundSet->AttackChooser->AttackEntries.Reset();
    for (FMCS_AttackEntry* Row : Rows)
        if (Row)
            FoundSet->AttackChooser->AttackEntries.Add(*Row);

    if (bDebug)
        UE_LOG(LogTemp, Log, TEXT("[CombatCore] Activated set: %s (%d attacks) Chooser: %s"),
            *NewAttackSetTag.ToString(),
            FoundSet->AttackChooser->AttackEntries.Num(),
            *FoundSet->AttackChooser->GetName());

    return true;
}

/**
 * Gets the currently active attack DataTable.
 */
UDataTable* UMCS_CombatCoreComponent::GetActiveAttackTable() const
{
    if (const FMCS_AttackSetData* Found = AttackSets.Find(ActiveAttackSetTag))
    {
        return Found->AttackDataTable;
    }
    return nullptr;
}

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
#include "GameFramework/CharacterMovementComponent.h"

#if WITH_EDITORONLY_DATA
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "Engine/Engine.h"
#endif


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

void UMCS_CombatCoreComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    // UpdatePlayerSituation(DeltaTime); // Uncomment if you want to update PlayerSituation every frame in C++. Can be done in Blueprint instead.
}

/*
 * Plays the selected attack's montage if valid
 * @param DesiredType - type of attack to perform
 * @param DesiredDirection - direction of the attack in world space
*/
void UMCS_CombatCoreComponent::PerformAttack(EMCS_AttackType DesiredType, EMCS_AttackDirection DesiredDirection, const FMCS_AttackSituation& CurrentSituation)
{
    if (!CurrentAttack.HasValidMontage())
    {
        if (!SelectAttack(DesiredType, DesiredDirection, CurrentSituation))
        {
            return;
        }
    }

    ACharacter* CharacterOwner = Cast<ACharacter>(GetOwner());
    if (!CharacterOwner || !CurrentAttack.HasValidMontage()) return;

    // Cache hitbox component reference
    CachedHitboxComp = CharacterOwner->FindComponentByClass<UMCS_CombatHitboxComponent>();

    // Bind notifies for the montage
    BindNotifiesForMontage(CurrentAttack.AttackMontage);

    // Retrieve anim instance
    UAnimInstance* AnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();
    if (!AnimInstance)
    {
        return;
    }

    //----------------------------------------
    // Smoothly blend between montages
    //----------------------------------------

    // Use designer-defined or default blend times
    float BlendInTime = FMath::Max(CurrentAttack.BlendInTime, 0.0f);
    float BlendOutTime = FMath::Max(CurrentAttack.BlendOutTime, 0.0f);

    // Use faster blend when chaining combos
    const bool bFromCombo = bIsComboWindowOpen;
    if (bFromCombo)
    {
        BlendInTime = FMath::Min(BlendInTime, 0.05f);
        BlendOutTime = FMath::Min(BlendOutTime, 0.05f);
    }

    // Smoothly fade out any active montage
    if (UAnimMontage* CurrentMontage = AnimInstance->GetCurrentActiveMontage())
    {
        if (CurrentMontage != CurrentAttack.AttackMontage)
        {
            AnimInstance->Montage_Stop(BlendOutTime, CurrentMontage);
        }
    }

    // Apply blend parameters to the new montage
    if (CurrentAttack.AttackMontage)
    {
        CurrentAttack.AttackMontage->BlendIn.SetBlendTime(BlendInTime);
        CurrentAttack.AttackMontage->BlendOut.SetBlendTime(BlendOutTime);
    }

    // Play the new montage with blending
    const float PlayRate = 1.0f;
    const float StartTime = 0.0f;
    AnimInstance->Montage_Play(CurrentAttack.AttackMontage, PlayRate, EMontagePlayReturnType::MontageLength, StartTime, true);

    // Jump to specified section if provided
    if (CurrentAttack.MontageSection != NAME_None)
    {
        AnimInstance->Montage_JumpToSection(CurrentAttack.MontageSection, CurrentAttack.AttackMontage);
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
        // UE_LOG(LogTemp, Error, TEXT("[CombatCore] No valid attack set or chooser for active tag: %s"), *ActiveAttackSetTag.ToString());
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

    // Cache current situation
    PlayerSituation = CurrentSituation;

    // Choose attack
    FMCS_AttackEntry ChosenAttack;
    const TArray<FMCS_AttackEntry> OriginalEntries = Chooser->AttackEntries;
    Chooser->AttackEntries = FilteredEntries;

    const bool bSuccess = Chooser->ChooseAttack(OwnerActor, Targets, DesiredDirection, CurrentSituation, ChosenAttack);
    Chooser->AttackEntries = OriginalEntries;

    if (bSuccess)
    {
        CurrentAttack = ChosenAttack;
    }

    return bSuccess;
}

/*
 * Attempts to chain into the next attack in a combo
 * @param DesiredType - type of attack to select
 * @param DesiredDirection - direction of the attack in world space
 */
bool UMCS_CombatCoreComponent::TryContinueCombo(EMCS_AttackType DesiredType, EMCS_AttackDirection DesiredDirection, const FMCS_AttackSituation& CurrentSituation)
{
    if (!bIsComboWindowOpen)
    {
        // UE_LOG(LogTemp, Log, TEXT("[CombatCore] Combo attempt ignored — window not active."));
        return false;
    }

    if (AllowedComboNames.IsEmpty())
    {
        // UE_LOG(LogTemp, Log, TEXT("[CombatCore] Combo attempt ignored — no valid follow-up attacks."));
        return false;
    }

    const FMCS_AttackSetData* ActiveSet = AttackSets.Find(ActiveAttackSetTag);
    if (!ActiveSet || !ActiveSet->AttackChooser) return false;

    UMCS_AttackChooser* Chooser = ActiveSet->AttackChooser;
    AActor* OwnerActor = GetOwnerActor();
    if (!OwnerActor) return false;

    // Filter by allowed combo names
    TArray<FMCS_AttackEntry> Filtered;
    for (const FMCS_AttackEntry& Entry : Chooser->AttackEntries)
    {
        if (AllowedComboNames.Contains(Entry.AttackName))
        {
            Filtered.Add(Entry);
        }
    }

    if (Filtered.IsEmpty())
    {
        // UE_LOG(LogTemp, Log, TEXT("[CombatCore] No matching combo follow-ups found."));
        return false;
    }

    // Temporarily override chooser entries
    const TArray<FMCS_AttackEntry> Original = Chooser->AttackEntries;
    Chooser->AttackEntries = Filtered;

    FMCS_AttackEntry NextAttack;
    bool bChosen = Chooser->ChooseAttack(OwnerActor, {}, DesiredDirection, CurrentSituation, NextAttack);

    Chooser->AttackEntries = Original;

    if (!bChosen)
    {
        // UE_LOG(LogTemp, Warning, TEXT("[CombatCore] Combo chooser failed to pick next attack."));
        return false;
    }

    // Chain into next attack
    CurrentAttack = NextAttack;
    PerformAttack(DesiredType, DesiredDirection, CurrentSituation);

    // UE_LOG(LogTemp, Log, TEXT("[CombatCore] Combo chained into attack: %s"), *NextAttack.AttackName.ToString());

    // Reset combo window state (will be reopened by next montage’s combo notify)
    bCanContinueCombo = false;
    bIsComboWindowOpen = false;
    AllowedComboNames.Reset();

    return true;
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
    // UE_LOG(LogTemp, Log, TEXT("[CombatCore] Target list changed: %d targets in range."), NewTargetCount);

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

void UMCS_CombatCoreComponent::BindNotifiesForMontage(UAnimMontage* Montage)
{
    UnbindAllNotifies();
    if (!Montage) return;

    for (const FAnimNotifyEvent& Event : Montage->Notifies)
    {
        if (Event.NotifyStateClass && Event.NotifyStateClass->GetClass()->IsChildOf(UAnimNotifyState_MCSHitboxWindow::StaticClass()))
        {
            if (UAnimNotifyState_MCSHitboxWindow* Notify = Cast<UAnimNotifyState_MCSHitboxWindow>(Event.NotifyStateClass))
            {
                // Avoid duplicate binds
                // Notify->OnNotifyBegin.RemoveDynamic(this, &UMCS_CombatCoreComponent::HandleHitboxNotifyBegin);
                // Notify->OnNotifyEnd.RemoveDynamic(this, &UMCS_CombatCoreComponent::HandleHitboxNotifyEnd);

                // Bind to hitbox notify events
                Notify->OnNotifyBegin.AddDynamic(this, &UMCS_CombatCoreComponent::HandleHitboxNotifyBegin);
                Notify->OnNotifyEnd.AddDynamic(this, &UMCS_CombatCoreComponent::HandleHitboxNotifyEnd);

                BoundHitboxNotifies.Add(Notify);
            }
        }
        else if (Event.NotifyStateClass && Event.NotifyStateClass->GetClass()->IsChildOf(UAnimNotifyState_MCSComboWindow::StaticClass()))
        {
            if (UAnimNotifyState_MCSComboWindow* Notify = Cast<UAnimNotifyState_MCSComboWindow>(Event.NotifyStateClass))
            {
                // Bind to combo notify events
                Notify->OnNotifyBegin.AddDynamic(this, &UMCS_CombatCoreComponent::HandleComboNotifyBegin);
                Notify->OnNotifyEnd.AddDynamic(this, &UMCS_CombatCoreComponent::HandleComboNotifyEnd);

                BoundComboNotifies.Add(Notify);
            }
        }
    }

    // UE_LOG(LogTemp, Log, TEXT("[CombatCore] Bound to %d hitbox notifies on montage %s"), BoundHitboxNotifies.Num(), *Montage->GetName());
}

void UMCS_CombatCoreComponent::UnbindAllNotifies()
{
    if (BoundHitboxNotifies.Num() == 0 && BoundComboNotifies.Num() == 0) return;

    // Unbind all hitbox notify events
    for (UAnimNotifyState_MCSHitboxWindow* NotifyCDO1 : BoundHitboxNotifies)
    {
        if (NotifyCDO1)
        {
            NotifyCDO1->OnNotifyBegin.RemoveDynamic(this, &UMCS_CombatCoreComponent::HandleHitboxNotifyBegin);
            NotifyCDO1->OnNotifyEnd.RemoveDynamic(this, &UMCS_CombatCoreComponent::HandleHitboxNotifyEnd);
        }
    }

    // Unbind all combo notify events
    for (UAnimNotifyState_MCSComboWindow* NotifyCDO2 : BoundComboNotifies)
    {
        if (NotifyCDO2)
        {
            NotifyCDO2->OnNotifyBegin.RemoveDynamic(this, &UMCS_CombatCoreComponent::HandleComboNotifyBegin);
            NotifyCDO2->OnNotifyEnd.RemoveDynamic(this, &UMCS_CombatCoreComponent::HandleComboNotifyEnd);
        }
    }

    // Clear the lists
    BoundHitboxNotifies.Reset();
    BoundComboNotifies.Reset();
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

    // UE_LOG(LogTemp, Log, TEXT("[CombatCore] Hitbox BEGIN (Start:%s End:%s R:%.1f)"), *Hitbox.StartSocket.ToString(), *Hitbox.EndSocket.ToString(), Hitbox.Radius);
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
        // UE_LOG(LogTemp, Log, TEXT("[CombatCore] Hitbox END (Label:%s)"), *Hitbox.StartSocket.ToString());
    }
}

void UMCS_CombatCoreComponent::HandleComboNotifyBegin()
{
    // Guard: only run if this character is actively playing this montage
    if (const ACharacter* C = Cast<ACharacter>(GetOwner());
        !(C && C->GetMesh() && C->GetMesh()->GetAnimInstance() &&
            C->GetMesh()->GetAnimInstance()->Montage_IsPlaying(CurrentAttack.AttackMontage))) return;

    // Mark combo window as active
    bIsComboWindowOpen = true;

    // Load the allowed next attacks for this attack
    AllowedComboNames = CurrentAttack.AllowedNextAttacks;
    bCanContinueCombo = AllowedComboNames.Num() > 0;

    // UE_LOG(LogTemp, Log, TEXT("[CombatCore] Combo Window BEGIN — %d allowed next attacks."), AllowedComboNames.Num());

    // Fire the combo begin event
    OnComboWindowBegin.Broadcast();
}

void UMCS_CombatCoreComponent::HandleComboNotifyEnd()
{
    // Guard: only run if this character is actively playing this montage
    if (const ACharacter* C = Cast<ACharacter>(GetOwner());
        !(C && C->GetMesh() && C->GetMesh()->GetAnimInstance() &&
            C->GetMesh()->GetAnimInstance()->Montage_IsPlaying(CurrentAttack.AttackMontage))) return;

    // Close combo window
    bIsComboWindowOpen = false;

    // UE_LOG(LogTemp, Log, TEXT("[CombatCore] Combo Window END."));

    // Fire the combo end event
    OnComboWindowEnd.Broadcast();

    // If combo was open but no input triggered next attack, reset
    if (!bCanContinueCombo)
    {
        AllowedComboNames.Reset();
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
        // UE_LOG(LogTemp, Warning, TEXT("[CombatCore] Unknown attack set tag: %s"), *NewAttackSetTag.ToString());
        return false;
    }

    if (!FoundSet->AttackDataTable || !FoundSet->AttackChooser)
    {
        // UE_LOG(LogTemp, Error, TEXT("[CombatCore] Attack set '%s' missing required DataTable or Chooser!"), *NewAttackSetTag.ToString());
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

    // UE_LOG(LogTemp, Log, TEXT("[CombatCore] Activated set: %s (%d attacks) Chooser: %s"),
        // *NewAttackSetTag.ToString(),
        // FoundSet->AttackChooser->AttackEntries.Num(),
        // *FoundSet->AttackChooser->GetName());

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

/**
 * Update Player Situation
 * @param DeltaTime - time since last update
 */
void UMCS_CombatCoreComponent::UpdatePlayerSituation(float DeltaTime)
{
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn) return;

    ACharacter* Character = Cast<ACharacter>(OwnerPawn);
    UCharacterMovementComponent* MoveComp = Character ? Character->GetCharacterMovement() : nullptr;

    // --- Basic Movement Flags ---
    PlayerSituation.bIsGrounded = MoveComp ? MoveComp->IsMovingOnGround() : true;
    PlayerSituation.bIsInAir = MoveComp ? MoveComp->IsFalling() : false;
    PlayerSituation.bIsRunning = MoveComp ? MoveComp->Velocity.Size() > 300.f : false;
    PlayerSituation.bIsCrouching = Character ? Character->bIsCrouched : false;

    // --- Advanced Combat States ---
    PlayerSituation.bIsCountering = false; // These can later be set by animation notifies or gameplay events
    PlayerSituation.bIsParrying = false;
    PlayerSituation.bIsRiposting = false;
    PlayerSituation.bIsFinishing = false;

    // --- Quantitative Stats ---
    PlayerSituation.Speed = MoveComp ? MoveComp->Velocity.Size() : 0.f;

    // Altitude = actor's height above the nearest ground trace
    FHitResult Hit;
    const FVector Start = OwnerPawn->GetActorLocation();
    const FVector End = Start - FVector(0.f, 0.f, 10000.f);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerPawn);
    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
        PlayerSituation.Altitude = (Start - Hit.Location).Size();
    else
        PlayerSituation.Altitude = 0.f;

    // Optional: get stamina/health percent from owner’s interface or attributes (placeholder)
    PlayerSituation.Stamina = 100.f;
    PlayerSituation.HealthPercent = 100.f;
}

#if WITH_EDITORONLY_DATA
/**
 * Draws the Motion Combat System debug overlay.
 * Call from your PlayerController::DrawHUD() or custom AHUD::DrawHUD().
 */
void UMCS_CombatCoreComponent::DrawDebugOverlay(FCanvas* Canvas, float& Y) const
{
    if (!Canvas) return;

    const FMCS_AttackSetData* ActiveSet = AttackSets.Find(ActiveAttackSetTag);
    if (!ActiveSet || !ActiveSet->AttackChooser) return;

    const TArray<FMCS_DebugAttackScore>& Scores = ActiveSet->AttackChooser->DebugScores;
    if (Scores.IsEmpty()) return;

    const float X = 50.f;
    const float LineHeight = 14.f;

    //----------------------------------------
    // Header
    //----------------------------------------
    {
        const FString Header = FString::Printf(
            TEXT("Motion Combat Debug - Active Set: %s"),
            *ActiveAttackSetTag.ToString());

        FCanvasTextItem HeaderItem(FVector2D(X, Y),
            FText::FromString(Header),
            GEngine->GetMediumFont(),
            FLinearColor(FColor::Cyan));

        HeaderItem.EnableShadow(FLinearColor::Black);
        Canvas->DrawItem(HeaderItem);
        Y += 22.f;
    }

    //----------------------------------------
    // Attack Entries
    //----------------------------------------
    for (const FMCS_DebugAttackScore& Info : Scores)
    {
        const FLinearColor Color =
            Info.bWasChosen ? FLinearColor::Yellow : FLinearColor::White;

        const FString Line = FString::Printf(
            TEXT("%s | Total: %.1f [B%.1f T%.1f D%.1f Dir%.1f Sit%.1f]"),
            *Info.AttackName.ToString(),
            Info.TotalScore,
            Info.BaseScore,
            Info.TagScore,
            Info.DistanceScore,
            Info.DirectionScore,
            Info.SituationScore);

        FCanvasTextItem LineItem(FVector2D(X, Y),
            FText::FromString(Line),
            GEngine->GetTinyFont(),
            Color);

        LineItem.EnableShadow(FLinearColor::Black);
        Canvas->DrawItem(LineItem);

        Y += LineHeight;
    }

    //----------------------------------------
    // Player Situation Summary
    //----------------------------------------
    Y += 20.f;
    {
        const FString SituationHeader = TEXT("Current Player Situation:");
        FCanvasTextItem SitHeader(FVector2D(X, Y),
            FText::FromString(SituationHeader),
            GEngine->GetMediumFont(),
            FLinearColor::Green);
        SitHeader.EnableShadow(FLinearColor::Black);
        Canvas->DrawItem(SitHeader);
        Y += 18.f;

        auto DrawBool = [ & ] (const FString& Label, bool bValue)
            {
                const FLinearColor Color = bValue ? FLinearColor::Green : FLinearColor::Red;
                const FString Line = FString::Printf(TEXT("%-12s : %s"), *Label, bValue ? TEXT("True") : TEXT("False"));
                FCanvasTextItem Item(FVector2D(X, Y),
                    FText::FromString(Line),
                    GEngine->GetTinyFont(),
                    Color);
                Canvas->DrawItem(Item);
                Y += LineHeight;
            };

        auto DrawFloat = [ & ] (const FString& Label, float Value)
            {
                const FString Line = FString::Printf(TEXT("%-12s : %.1f"), *Label, Value);
                FCanvasTextItem Item(FVector2D(X, Y),
                    FText::FromString(Line),
                    GEngine->GetTinyFont(),
                    FLinearColor::Yellow);
                Canvas->DrawItem(Item);
                Y += LineHeight;
            };

        // Qualitative flags
        DrawBool(TEXT("Is Grounded"), PlayerSituation.bIsGrounded);
        DrawBool(TEXT("Is In Air"), PlayerSituation.bIsInAir);
        DrawBool(TEXT("Is Running"), PlayerSituation.bIsRunning);
        DrawBool(TEXT("Is Crouching"), PlayerSituation.bIsCrouching);
        DrawBool(TEXT("Is Countering"), PlayerSituation.bIsCountering);
        DrawBool(TEXT("Is Parrying"), PlayerSituation.bIsParrying);
        DrawBool(TEXT("Is Riposting"), PlayerSituation.bIsRiposting);
        DrawBool(TEXT("Is Finishing"), PlayerSituation.bIsFinishing);

        Y += 5.f;

        // Quantitative values
        DrawFloat(TEXT("Speed"), PlayerSituation.Speed);
        DrawFloat(TEXT("Altitude"), PlayerSituation.Altitude);
        DrawFloat(TEXT("Stamina"), PlayerSituation.Stamina);
        DrawFloat(TEXT("Health %"), PlayerSituation.HealthPercent);
    }
}
#endif

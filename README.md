# Motion Combat System (MCS)
Motion Combat System (MCS) is a modular, data-driven melee combat framework for Unreal Engine 5.6+, inspired by the Motion Matching system. 

Instead of hardcoding combos or animation chains, MCS leverages data tables, targeting subsystems, and a customizable chooser to dynamically select the most suitable attack at runtime — based on player intent, enemy proximity, and combat context. 

This system is designed to give designers complete control via Blueprints and DataTables, while still providing a robust C++ foundation for developers who want to extend or optimize the behavior. 

It aims to make combat fluid, reactive, and extensible, whether you're building a soulslike, action RPG, or stylized brawler.

## Compile Version:
5.6.1

***
⚔️ Provided by God’s Studio — freely for you to use, learn from, and build upon.
"I hope you learn from this as I learned creating it."
***

## ⚙️ Combat System Architecture
MCS is built from five interconnected systems that together form a flexible foundation for adaptive melee combat.

## UMCS_TargetingSubsystem
**Type:** UWorldSubsystem

**Purpose:** World-level manager for discovering and tracking combat targets.

This subsystem automatically scans the environment at set intervals, detecting all valid combatants that implement the UMCS_CombatTargetInterface. It maintains a live registry of nearby enemies and removes out-of-range or destroyed targets.

By existing at the world level, the subsystem ensures consistent combat awareness across all actors in the level.

**Key Responsibilities:**

- Periodically scans for actors implementing IMCS_CombatTargetInterface.
- Filters out invalid or distant targets dynamically.
- Exposes helper functions like GetClosestTarget() and GetAllTargets().
- Provides optional debug drawing for target visualization.

**Example Usage:**
```
UMCS_TargetingSubsystem* TargetSys = World->GetSubsystem<UMCS_TargetingSubsystem>();
AActor* Closest = TargetSys->GetClosestTarget(Player->GetActorLocation());
```

## FMCS_AttackEntry
**Type:** FTableRowBase (DataTable Struct)

**Purpose:** Defines individual attacks used by the Chooser and Core Component.

Each FMCS_AttackEntry represents a single attack configuration, stored in a DataTable.

This includes metadata, montage references, range windows, and selection weights — allowing designers to create dozens of unique attacks without touching code.

**Core Properties:**
- AttackName – Designer-friendly identifier.
- AttackType – Enum category (Light, Heavy, Special, etc.).
- AttackMontage – Animation montage reference.
- RangeStart / RangeEnd – Effective range window for selection.
- SelectionWeight – Bias for the Chooser’s scoring.
- AttackTag – Integrates with GameplayAbilitySystem (GAS).

**Use Case:**

A row defines an attack like “Heavy Overhead Swing” with a 300–600 range and a higher selection weight than “Light Jab.”

## UMCS_CombatTargetInterface
**Type:** UInterface

**Purpose:** Marks an actor as being targetable by the Motion Combat System.

Any actor implementing this interface can participate in combat targeting.

The single function CanBeTargeted() lets enemies dynamically opt in or out of detection (for instance, when dead or invulnerable).

**Key Benefit:**
Keeps targeting lightweight and modular — you only need to add this interface to any pawn or AI to make it combat-visible.

```
UCLASS()
class AMyEnemyCharacter : public ACharacter, public IMCS_CombatTargetInterface
{
    GENERATED_BODY()
public:
    virtual bool CanBeTargeted_Implementation() const override { return !bIsDead; }
};
```

## UMCS_AttackChooser
**Type:** UObject

**Purpose:** Selects the best attack entry based on context and scoring heuristics.

The **Attack Chooser** is the heart of the Motion Combat System’s intelligence.
It takes a list of FMCS_AttackEntry structs and scores them dynamically based on **distance**, **angle**, and **custom logic** (which can be overridden in Blueprints).

**Default Behavior:**

- Filters by distance and angle constraints.
- Calculates a deterministic score based on attack metadata.
- Supports random tie-breaking for variety.

**Designer Power:**
Override ScoreAttack() in Blueprints to implement project-specific heuristics, such as stamina availability, attack cooldowns, or combo sequencing.

**Example Blueprint Extension:**
```
float ScoreAttack_Implementation(const FMCS_AttackEntry& Entry, AActor* Instigator, const TArray<AActor*>& Targets) const override
{
    return Entry.SelectionWeight * (IsTargetInRange(Targets) ? 2.f : 1.f);
}
```

## UMCS_CombatCoreComponent
**Type:** UActorComponent

**Purpose:** Central orchestrator for loading attacks, selecting, and executing them.

**Responsibilities:**

- Load all FMCS_AttackEntry rows from the DataTable.
- Maintain a reference to the UMCS_TargetingSubsystem.
- Use the UMCS_AttackChooser to select the optimal attack.
- Execute the selected attack montage.

**Example Flow:**
```
// In your PlayerController input binding
MCSCoreComponent->SelectAttack();
MCSCoreComponent->ExecuteAttack();
```

**Debug Logging Example:**
```
[CombatCore] Selected attack: HeavySlash (Damage=50.0, Range=400)
[CombatCore] Playing montage: M_Attack_HeavySlash
```

# 🧩 Setup
Coming soon — setup guide will include instructions for integrating the MCS plugin into your project, configuring DataTables, and wiring up player input to perform attacks.
***

# 🏗️ FAB Content Used
- Paragon: Twinblast (https://www.fab.com/listings/9fa88852-5711-42e1-94fa-2491498a64da)

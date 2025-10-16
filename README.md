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


## FMCS_AttackEntry

## UMCS_CombatTargetInterface

## UMCS_AttackChooser

## UMCS_CombatCoreComponent

# Setup
TBD
***

# FAB Content Used
- Paragon: Twinblast (https://www.fab.com/listings/9fa88852-5711-42e1-94fa-2491498a64da)
- 

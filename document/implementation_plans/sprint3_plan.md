# Sprint 3: Enemy AI Implementation Plan

This plan details the implementation of the enemy logic and AI as outlined in the Sprint 3 requirements, specifically addressing the `Enemy` base class, state machine (`EnemyState`), and the concrete enemy types (`Snake`, `Bat`, `Spider`, and `NemesisGhost`).

## Proposed Changes

### Core Enemy Architecture
#### [NEW] [Enemy.h](file:///d:/CS202-Cavern-Descent/src/entities/Enemy.h)
#### [NEW] [Enemy.cpp](file:///d:/CS202-Cavern-Descent/src/entities/Enemy.cpp)
Create an abstract `Enemy` class extending `DynamicEntity`.
- **Properties**: `health`, `damage`, `EnemyState` (enum: IDLE, CHASE, RETURN).
- **Methods**: Virtual `update(float dt)` to handle state transitions and movement logic. 
- **State Machine (Advanced Feature 2)**: 
  - `IDLE`: Default patrolling or waiting.
  - `CHASE`: Triggered when the player is within aggro range.
  - `RETURN`: Triggered when the player leaves the aggro range; enemy returns to spawn.

### Specific Enemy Implementations
#### [NEW] [Snake.h](file:///d:/CS202-Cavern-Descent/src/entities/Snake.h) & [Snake.cpp](file:///d:/CS202-Cavern-Descent/src/entities/Snake.cpp)
- **Behavior**: Patrols back and forth. Reverses direction when hitting a wall or reaching a ledge (no `CHASE` state needed, just simple patrol `IDLE`).

#### [NEW] [Bat.h](file:///d:/CS202-Cavern-Descent/src/entities/Bat.h) & [Bat.cpp](file:///d:/CS202-Cavern-Descent/src/entities/Bat.cpp)
- **Behavior**: `IDLE` (Hanging from ceiling/air). Transitions to `CHASE` when the player is nearby, flying directly towards the player, ignoring gravity.

#### [NEW] [Spider.h](file:///d:/CS202-Cavern-Descent/src/entities/Spider.h) & [Spider.cpp](file:///d:/CS202-Cavern-Descent/src/entities/Spider.cpp)
- **Behavior**: `IDLE` (Hanging on ceiling). When player is underneath, drops down. Once on the ground, periodically jumps towards the player.

#### [NEW] [NemesisGhost.h](file:///d:/CS202-Cavern-Descent/src/entities/NemesisGhost.h) & [NemesisGhost.cpp](file:///d:/CS202-Cavern-Descent/src/entities/NemesisGhost.cpp)
- **Behavior (Advanced Feature 9)**: Spawns after a set time limit on the floor (managed by LevelManager). Passes through terrain (ignores standard AABB tile collision). Constantly in `CHASE` mode directly tracking the player.

### Integration
#### [MODIFY] [EntityFactory.cpp](file:///d:/CS202-Cavern-Descent/src/entities/EntityFactory.cpp)
- Replace the temporary `VisualEnemy` instantiations with the new `Snake`, `Bat`, `Spider`, and `NemesisGhost` classes.

## User Review Required

> [!WARNING]
> **Entity Update Signature vs Global Access**
> `Entity::update(float dt)` is currently defined in `Entity.h`. Enemies need to know the Player's position to execute `CHASE` logic. 
> To adhere to "No dynamic_cast" and clean architecture, I will have the `Enemy` fetch the player from a global context (e.g., `GameManager::getInstance().getPlayer()`) rather than modifying the base `update()` signature. Let me know if you prefer modifying the signature instead.

> [!IMPORTANT]
> **Nemesis Ghost Spawning**
> The ghost needs a timer. I will implement the ghost timer logic inside `GameManager` or `LevelGenerator`. Once the timer expires, it will instantiate a `NemesisGhost` at the top of the level and add it to the enemy vector.

Please approve this plan and provide feedback on the Open Questions!

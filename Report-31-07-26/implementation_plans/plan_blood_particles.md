# Add Blood Particles

This plan outlines the addition of a new `Particle` system for blood drops when entities (Player or Enemies) take damage, as well as an EventBus integration to spawn them.

## Proposed Changes

### `src/entities/Particle.h` & `src/entities/Particle.cpp`
- **[NEW]** Create a `Particle` class inheriting from `DynamicEntity`.
- **Constructor:** Takes position, initial velocity (for a random burst effect), lifetime, and an initial frame (from `gfx_blood_rock_rope_poof.png`).
- **Update:** Applies gravity (using `DynamicEntity::applyGravity`), moves, and stops moving if it hits the ground. It also ticks down its lifetime timer.
- **Render:** Renders the blood droplet sprite. As `lifetime` approaches 0, it fades out by adjusting the tint alpha.

### `src/entities/EntityFactory.h` & `src/entities/EntityFactory.cpp`
- **[MODIFY]** Add a `createBloodParticle(float x, float y)` method.
- This method will spawn a `Particle`, assigning it a random velocity vector, a random blood droplet frame (columns 0 to 5 of the spritesheet), and a random lifetime (~1.0s to 3.0s).

### `src/core/GameState.cpp` (PlayState)
- **[MODIFY]** In `PlayState::enter()`, subscribe to `EVENT_PLAYER_DAMAGED` and `EVENT_ENEMY_DAMAGED`. When received, call `EntityFactory::createBloodParticle` multiple times (e.g., 5-8 particles) at the given `worldX`, `worldY` and push them into `pendingEntities`.

### `src/player/Player.cpp` & `src/entities/enemies/Enemy.cpp`
- **[MODIFY]** In `takeDamage`, set `data.worldX` and `data.worldY` to the center of the entity before publishing the damage event, so `PlayState` knows where to spawn the blood.

## Open Questions

- Should blood particles bounce when they hit the floor, or should they just splat and stop moving? (Defaulting to splat and stop to save on physics calculations).

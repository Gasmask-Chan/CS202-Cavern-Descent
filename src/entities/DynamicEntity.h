#pragma once

#include "Entity.h"

namespace Platformer {

class DynamicEntity : public Entity {
protected:
    float vx;
    float vy;
    float gravity;
    bool isGrounded;
    bool isFacingRight;

public:
    DynamicEntity(float x, float y, float w, float h);

    /**
     * @brief If `!isGrounded`, adds `gravity * dt to vy`. Gravity constant is ~800 pixels/sec². `isGrounded` is set to `true` by `PhysicsSystem` when a downward collision is resolved. Reset to `false` at the start of each frame.
     * 
     * @param dt 
     */
    void applyGravity(float dt);

    /**
     * @brief Adds `dx` to `x` and `dy` to `y`. Raw position change — no collision checking. Collision is handled separately by `PhysicsSystem::resolveEntityTileCollision()`.
     * 
     * @param dx 
     * @param dy 
     */
    void move(float dx, float dy);

    /**
     * @brief Directly sets velocity components. Used by knockback, bounce, and state transitions (e.g., `ChaseState` sets `vx` toward player).
     * 
     * @param vx 
     * @param vy 
     */
    void setVelocity(float vx, float vy);

    float getVelocityX();

    float getVelocityY();
};

}

#pragma once

#include "../Config.h"

namespace Platformer {

class Entity {
protected:
    float x;
    float y;
    float width;
    float height;
    bool isActive;
    Texture2D sprite;

public:
    Entity(float x, float y, float w, float h);

    virtual ~Entity();

    /**
     * @brief Virtual. Base implementation is empty. Subclasses override to add per-frame logic (movement, AI, animation).
     * 
     * @param dt 
     */
    virtual void update(float dt);

    /**
     * @brief Virtual. Draws sprite texture at `(x, y)` tinted by `lightLevel` (0.0=black, 1.0=full brightness) using Raylib `DrawTextureEx` with `ColorTint`.
     * 
     * @param lightLevel 
     */
    virtual void render(float lightLevel);

    /**
     * @brief Returns a Raylib `Rectangle{x, y, width, height}` representing the axis-aligned bounding box. Used by `PhysicsSystem` for all collision checks.
     * 
     * @return Rectangle 
     */
    Rectangle getAABB();

    /**
     * @brief Returns `isActive`. Entities with `isActive == false` are removed during the cleanup step (step 20 in game loop).
     * 
     * @return true 
     * @return false 
     */
    bool isAlive();

    /**
     * @brief Sets `isActive = false`. The entity remains in its vector until `LevelManager::removeDeadEntities()` erases it.
     * 
     */
    void destroy();

    float getX();
    
    float getY();
};

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
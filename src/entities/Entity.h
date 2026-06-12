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



}
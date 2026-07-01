#pragma once

#include "../Config.h"
#include "../entities/DynamicEntity.h"
#include "../level/TileMap.h"

namespace Platformer {

struct CollisionResult {
    bool collided;
    float contactTime;
    Vector2 contactNormal;
    Vector2 contactPoint;
};

class PhysicsSystem {
private:
    TileMap* tileMap;

public:
    PhysicsSystem(TileMap* map);

    /**
     * @brief Predicts the entity's next position from its velocity. Checks all tiles overlapping the predicted `AABB`. For each collision: calculates penetration depth, pushes entity out along the axis of least penetration (`X` or `Y`). If pushed out vertically downward, sets `e->isGrounded = true`. If pushed out upward (hit ceiling), sets `e->vy = 0`.
     * 
     * @param e 
     */
    void resolveEntityTileCollision(DynamicEntity* e);

    /**
     * @brief Returns `true` if rectangles `a` and `b` overlap. Standard axis-aligned test. Used for all entity-entity collision checks in the O(N^2) loop.
     * 
     * @param a 
     * @param b 
     * @return true 
     * @return false 
     */
    bool checkAABBOverlap(Rectangle a, Rectangle b);

    /**
     * @brief Continuous collision detection. Projects entity's movement over `dt` and finds the earliest contact time (`0.0` to `1.0`) with any solid tile. Returns `CollisionResult` with `contactTime`, `contactNormal`, and `contactPoint`. Used for fast-moving entities to prevent tunneling through thin walls.
     * 
     * @param e 
     * @param dt 
     * @return CollisionResult 
     */
    CollisionResult sweepAABB(DynamicEntity* e, float dt);
};

}

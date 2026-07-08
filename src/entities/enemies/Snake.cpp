#include "Snake.h"
#include <raylib.h>
#include <cstdio>

namespace Platformer {

Snake::Snake(float x, float y, float w, float h)
    : Enemy(x, y, w, h, 1, 1), waitTimer(0.0f), moveSpeed(40.0f) {
    direction = (GetRandomValue(0, 1) == 0) ? -1 : 1;
    vx = direction * moveSpeed;
    isFacingRight = (direction == 1);
    setAnimation(4, 0.2f, 0, 1); // 4 frames, 0.2s speed, base_x=0, base_y=1 (row 1)
}

Snake::~Snake() {}

void Snake::update(float dt, Player* player) {
    // Call Enemy::update to process animation and base logic first
    Enemy::update(dt, player);

    // Ensure animation is set for IDLE state (walking)
    setAnimation(4, 0.2f, 0, 1);
    applyGravity(dt);
    
    if (waitTimer > 0.0f) {
        waitTimer -= dt;
        if (waitTimer <= 0.0f) {
            // Just finished waiting
            vx = moveSpeed * direction;
        } else {
            vx = 0; // Stop moving while waiting
        }
    } else {
        // If we were supposed to be moving, but vx was forced to 0 by PhysicsSystem last frame,
        // it means we hit a wall.
        if (vx == 0) {
            waitTimer = GetRandomValue(50, 150) / 100.0f; // Wait 0.5s to 1.5s
            direction *= -1; // Turn around
            isFacingRight = (direction == 1);
        } else {
            vx = moveSpeed * direction;
            isFacingRight = (direction == 1);
        }
    }
    
    move(vx * dt, vy * dt);
}

}

#include "Snake.h"
#include "../../level/TileMap.h"
#include "../../liquid/LiquidSimulator.h"
#include <raylib.h>
#include <cstdio>

namespace Platformer {

Snake::Snake(float x, float y, float w, float h)
    : Enemy(x, y, w, h, 1, 1), waitTimer(0.0f), moveSpeed(40.0f) {
    changeState(Enemy::idleState);
    direction = (GetRandomValue(0, 1) == 0) ? -1 : 1;
    vx = direction * moveSpeed;
    isFacingRight = (direction == 1);
    setAnimation(4, 0.2f, 0, 1); // 4 frames, 0.2s speed, base_x=0, base_y=1 (row 1)
}

Snake::~Snake() {}

void Snake::update(float dt, Player* player) {
    // Call Enemy::update to process state and base logic first
    Enemy::update(dt, player);
    
    applyGravity(dt);
    move(vx * dt, vy * dt);
}

void Snake::handleIdle(float dt, Player* player) {
    // Ensure animation is set for IDLE state (walking)
    setAnimation(4, 0.2f, 0, 1);
    
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
        bool hitWall = (vx == 0);
        bool ledgeAhead = false;
        bool waterAhead = false;
        
        if (tileMap && !hitWall) {
            // Check ledge ahead
            // Snake's center is x + width/2. We check one tile ahead based on direction.
            int nextGridX = (x + width / 2.0f + direction * 16.0f) / 32;
            int nextGridY = (y + height + 2.0f) / 32; // Just below the snake
            if (!tileMap->isSolid(nextGridX, nextGridY)) {
                ledgeAhead = true;
            }
            
            // Check water ahead
            if (liquidSim) {
                Rectangle nextAABB = {x + direction * 16.0f, y, width, height};
                if (liquidSim->isWaterAt(nextAABB)) {
                    waterAhead = true;
                }
            }
        }

        if (hitWall || ledgeAhead || waterAhead) {
            waitTimer = GetRandomValue(50, 150) / 100.0f; // Wait 0.5s to 1.5s
            direction *= -1; // Turn around
            isFacingRight = (direction == 1);
            vx = 0;
        } else {
            vx = moveSpeed * direction;
            isFacingRight = (direction == 1);
        }
    }
}

void Snake::handleChase(float dt, Player* player) {
    // Snake doesn't chase
    changeState(Enemy::idleState);
}

void Snake::handleReturn(float dt, Player* player) {
    // Snake doesn't return
    changeState(Enemy::idleState);
}

}

#include "Spider.h"
#include "../player/Player.h"
#include <raylib.h>
#include <cmath>

namespace Platformer {

Spider::Spider(float x, float y, float w, float h)
    : Enemy(x, y, w, h, 1, 1), jumpTimer(0.0f) {
    currentState = EnemyState::IDLE;
    gravity = 0; // Don't fall initially
}

Spider::~Spider() {}

void Spider::update(float dt, Player* player) {
    Enemy::update(dt, player);
    
    if (!player) return;
    
    float dx = player->getX() - x;
    float dy = player->getY() - y;
    
    if (currentState == EnemyState::IDLE) {
        vx = 0;
        vy = 0;
        
        // Check if player is directly below (within 2 tiles horizontally) or generally close
        if (dy > 0 && std::abs(dx) < 64.0f) {
            currentState = EnemyState::CHASE;
            gravity = 800.0f; // Enable gravity so it falls
        }
    } else if (currentState == EnemyState::CHASE) {
        applyGravity(dt);
        
        if (isGrounded) {
            if (jumpTimer > 0.0f) {
                jumpTimer -= dt;
                vx = 0; // Stop moving horizontally when grounded and waiting
            } else {
                // Jump towards player with random force
                jumpTimer = GetRandomValue(100, 250) / 100.0f; // 1.0 to 2.5 seconds between jumps
                
                float jumpVy = -(GetRandomValue(300, 600)); // Random height
                float jumpVx = (dx > 0) ? GetRandomValue(100, 300) : -GetRandomValue(100, 300); // Random distance towards player
                
                vx = jumpVx;
                vy = jumpVy;
                isGrounded = false;
                isFacingRight = (vx > 0);
            }
        }
    }
    
    move(vx * dt, vy * dt);
}

}

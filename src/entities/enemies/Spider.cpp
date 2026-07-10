#include "Spider.h"
#include "../../player/Player.h"
#include <raylib.h>
#include <cmath>

namespace Platformer {

Spider::Spider(float x, float y, float w, float h)
    : Enemy(x, y, w, h, 1, 1), jumpTimer(0.0f) {
    changeState(Enemy::idleState);
    gravity = 0; // Don't fall initially
    setAnimation(1, 0.2f, 0, 0); // Hanging
}

Spider::~Spider() {}

void Spider::update(float dt, Player* player) {
    Enemy::update(dt, player);
    
    move(vx * dt, vy * dt);
}

void Spider::handleIdle(float dt, Player* player) {
    vx = 0;
    vy = 0;
    setAnimation(1, 0.2f, 0, 0); // Hanging
    
    if (!player) return;
    
    float dx = player->getX() - x;
    float dy = player->getY() - y;
    
    // Check if player is directly below (within 2 tiles horizontally) or generally close
    if (dy > 0 && std::abs(dx) < 64.0f) {
        changeState(Enemy::chaseState);
        gravity = 800.0f; // Enable gravity so it falls
    }
}

void Spider::handleChase(float dt, Player* player) {
    applyGravity(dt);
    
    if (!player) return;
    float dx = player->getX() - x;
    
    if (isGrounded) {
        if (jumpTimer > 0.0f) {
            setAnimation(1, 0.2f, 0, 0); // Ground idle
            jumpTimer -= dt;
            vx = 0; // Stop moving horizontally when grounded and waiting
        } else {
            setAnimation(3, 0.1f, 1, 0); // Jumping
            // Jump towards player with random force
            jumpTimer = GetRandomValue(100, 250) / 100.0f; // 1.0 to 2.5 seconds between jumps
            
            float jumpVy = -(GetRandomValue(300, 600)); // Random height
            float jumpVx = (dx > 0) ? GetRandomValue(100, 300) : -GetRandomValue(100, 300); // Random distance towards player
            
            if (jumpVx > 0) isFacingRight = true;
            else if (jumpVx < 0) isFacingRight = false;
            
            vx = jumpVx;
            vy = jumpVy;
            isGrounded = false;
            isFacingRight = (vx > 0);
        }
    }
}

void Spider::handleReturn(float dt, Player* player) {
    // Spider does not return, it just chases forever once detached
    changeState(Enemy::chaseState);
}

}

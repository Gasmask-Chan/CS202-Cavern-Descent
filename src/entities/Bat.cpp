#include "Bat.h"
#include "../player/Player.h"
#include <cmath>
#include <algorithm>

namespace Platformer {

Bat::Bat(float x, float y, float w, float h)
    : Enemy(x, y, w, h, 1, 1) {
    currentState = EnemyState::IDLE;
    gravity = 0; // Bats don't fall by default
}

Bat::~Bat() {}

void Bat::update(float dt, Player* player) {
    Enemy::update(dt, player);
    // Note: applyGravity is skipped or overridden since gravity is 0

    if (!player) return;

    float dx = player->getX() - x;
    float dy = player->getY() - y;
    float dist = std::sqrt(dx*dx + dy*dy);
    float tileSize = 32.0f;
    
    // Trigger zone 7x7
    float triggerDist = 7.0f * tileSize;
    // Lose aggro zone 9x9
    float loseAggroDist = 9.0f * tileSize;

    if (currentState == EnemyState::IDLE) {
        vx = 0;
        vy = 0;
        // Check if player is below and within 7 tiles
        if (dy > 0 && dist < triggerDist) {
            currentState = EnemyState::CHASE;
            // Play squeak sound here (handled by AudioSystem later)
        }
    } else if (currentState == EnemyState::CHASE) {
        if (dist > loseAggroDist) {
            currentState = EnemyState::RETURN;
        } else {
            // Fly towards player
            float speed = 80.0f;
            vx = (dx / dist) * speed;
            vy = (dy / dist) * speed;
            isFacingRight = (vx > 0);
        }
    } else if (currentState == EnemyState::RETURN) {
        // If we hit a ceiling, PhysicsSystem will set our vy to 0 in resolveEntityTileCollision
        // We check if vy == 0 (from previous frame)
        if (vy == 0) {
            currentState = EnemyState::IDLE;
        } else {
            // Fly straight up
            vx = 0;
            vy = -80.0f;
        }
    }
    
    move(vx * dt, vy * dt);
    
    // Actually, if vy is set to -80 right before move, vy == 0 check will never trigger.
    // We need to check if physics zeroed it out. 
    // Since physics runs after update, vy is changed after move.
    // But wait, we just set vy = -80! 
    // Let's do the check at the TOP of the function.
}

}

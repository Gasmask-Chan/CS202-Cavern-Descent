#include "NemesisGhost.h"
#include "../player/Player.h"
#include <cmath>

namespace Platformer {

NemesisGhost::NemesisGhost(float x, float y, float w, float h)
    : Enemy(x, y, w, h, 999, 1) { 
    currentState = EnemyState::CHASE;
    gravity = 0; 
    passesThroughWalls = true; // Set flag to true so PhysicsSystem ignores it
}

NemesisGhost::~NemesisGhost() {}

void NemesisGhost::update(float dt, Player* player) {
    Enemy::update(dt, player);
    
    if (!player) return;
    
    float dx = player->getX() - x;
    float dy = player->getY() - y;
    float dist = std::sqrt(dx*dx + dy*dy);
    
    if (dist > 0.0f) {
        float speed = 60.0f; // Relentless pursuit
        vx = (dx / dist) * speed;
        vy = (dy / dist) * speed;
        isFacingRight = (vx > 0);
    }
    
    move(vx * dt, vy * dt);
}

}

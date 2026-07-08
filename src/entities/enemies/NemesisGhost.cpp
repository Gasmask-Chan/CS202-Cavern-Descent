#include "NemesisGhost.h"
#include "../../player/Player.h"
#include <cmath>

namespace Platformer {

NemesisGhost::NemesisGhost(float x, float y, float w, float h)
    : Enemy(x, y, w, h, 999, 1) { 
    changeState(Enemy::chaseState);
    gravity = 0; 
    passesThroughWalls = true; // Set flag to true so PhysicsSystem ignores it
    setAnimation(1, 0.2f, 0, 0); // Default to 1 frame for now
}

NemesisGhost::~NemesisGhost() {}

void NemesisGhost::update(float dt, Player* player) {
    Enemy::update(dt, player);
    move(vx * dt, vy * dt);
}

void NemesisGhost::handleIdle(float dt, Player* player) {
    // Nemesis ghost doesn't idle
    changeState(Enemy::chaseState);
}

void NemesisGhost::handleChase(float dt, Player* player) {
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
}

void NemesisGhost::handleReturn(float dt, Player* player) {
    // Nemesis ghost doesn't return
    changeState(Enemy::chaseState);
}

}

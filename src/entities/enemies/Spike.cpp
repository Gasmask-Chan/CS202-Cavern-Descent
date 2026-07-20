#include "Spike.h"
#include "../../player/Player.h"

namespace Platformer {

Spike::Spike(float x, float y, float w, float h)
    : Enemy(x, y, w, h, 999, 1), _blood(false) {
    changeState(Enemy::idleState);
    gravity = 0; // Spikes don't fall
    passesThroughWalls = true; // Prevents PhysicsSystem from pushing the spike up out of the ground
}

Spike::~Spike() {}

void Spike::update(float dt, Player* player) {
    Enemy::update(dt, player);
    
    if (player && player->isAlive()) {
        Rectangle pAABB = player->getAABB();
        Rectangle myAABB = getAABB();
        
        // Spike kill condition: player falling onto the spike
        if (CheckCollisionRecs(pAABB, myAABB)) {
            // instant kill only if player is falling (y_speed > 1) and not standing
            if (!player->getIsGrounded() && player->getVelocityY() > 10.0f) { // 10.0f to be safe against slight jumps but trigger on small falls
                player->takeDamage(100);
                setBlood();
            }
        }
    }
}

void Spike::setBlood() {
    _blood = true;
}

void Spike::updateSpriteRect() {
    if (sprite.id != 0) {
        if (_blood) {
            srcRect = { 16.0f, 0.0f, 16.0f, 16.0f }; // Bloody spike
        } else {
            srcRect = { 0.0f, 0.0f, 16.0f, 16.0f }; // Clean spike
        }
    }
}

void Spike::handleIdle(float dt, Player* player) {}
void Spike::handleChase(float dt, Player* player) {}
void Spike::handleReturn(float dt, Player* player) {}

}

#include "Bat.h"
#include "../../player/Player.h"
#include <cmath>
#include <algorithm>

namespace Platformer {

Bat::Bat(float x, float y, float w, float h)
    : Enemy(x, y, w, h, 1, 1) {
    changeState(Enemy::idleState);
    gravity = 0; // Bats don't fall by default
    setAnimation(1, 0.2f, 0, 0); // IDLE (hanging)
}

Bat::~Bat() {}

void Bat::update(float dt, Player* player) {
    // If we hit a ceiling while returning, physics sets vy = 0.
    // We check this here before update() might override it.
    if (currentStateObj == Enemy::returnState && vy == 0) {
        changeState(Enemy::idleState);
    }

    Enemy::update(dt, player);
    
    // Physics handles movement at the end
    move(vx * dt, vy * dt);
}

void Bat::handleIdle(float dt, Player* player) {
    vx = 0;
    vy = 0;
    setAnimation(1, 0.2f, 0, 0); // Hanging

    if (!player) return;
    float dx = player->getX() - x;
    float dy = player->getY() - y;
    float dist = std::sqrt(dx*dx + dy*dy);
    float triggerDist = 7.0f * 32.0f;

    if (dy > 0 && dist < triggerDist) {
        changeState(Enemy::chaseState);
    }
}

void Bat::handleChase(float dt, Player* player) {
    setAnimation(4, 0.1f, 1, 0); // Flying

    if (!player) return;
    float dx = player->getX() - x;
    float dy = player->getY() - y;
    float dist = std::sqrt(dx*dx + dy*dy);
    float loseAggroDist = 9.0f * 32.0f;

    if (dist > loseAggroDist) {
        changeState(Enemy::returnState);
    } else {
        float speed = 80.0f;
        vx = (dx / dist) * speed;
        vy = (dy / dist) * speed;
        isFacingRight = (vx > 0);
    }
}

void Bat::handleReturn(float dt, Player* player) {
    setAnimation(4, 0.1f, 1, 0); // Flying
    vx = 0;
    vy = -80.0f; // Fly straight up
}

}

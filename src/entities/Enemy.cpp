#include "Enemy.h"

namespace Platformer {

Enemy::Enemy(float x, float y, float w, float h, int hp, int dmg)
    : DynamicEntity(x, y, w, h), health(hp), damage(dmg), currentState(EnemyState::IDLE) {
}

Enemy::~Enemy() {}

void Enemy::update(float dt, Player* player) {
    DynamicEntity::update(dt, player);
}

int Enemy::getHealth() const { return health; }
int Enemy::getDamage() const { return damage; }
EnemyState Enemy::getState() const { return currentState; }

void Enemy::takeDamage(int dmg) {
    health -= dmg;
    if (health <= 0) {
        destroy();
    }
}

}

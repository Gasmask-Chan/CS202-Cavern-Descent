#include "EnemyState.h"
#include "Enemy.h"

namespace Platformer {

// -----------------------------------------------------------------------------
// IdleState
// -----------------------------------------------------------------------------
void IdleState::enter(Enemy* enemy) {
    // We can leave this empty, or allow enemy to hook into it later
}

void IdleState::update(Enemy* enemy, float dt, Player* player) {
    enemy->handleIdle(dt, player);
}

void IdleState::exit(Enemy* enemy) {
}

// -----------------------------------------------------------------------------
// ChaseState
// -----------------------------------------------------------------------------
void ChaseState::enter(Enemy* enemy) {
}

void ChaseState::update(Enemy* enemy, float dt, Player* player) {
    enemy->handleChase(dt, player);
}

void ChaseState::exit(Enemy* enemy) {
}

// -----------------------------------------------------------------------------
// ReturnState
// -----------------------------------------------------------------------------
void ReturnState::enter(Enemy* enemy) {
}

void ReturnState::update(Enemy* enemy, float dt, Player* player) {
    enemy->handleReturn(dt, player);
}

void ReturnState::exit(Enemy* enemy) {
}

}
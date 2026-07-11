#pragma once

#include <memory>

namespace Platformer {

class Enemy;
class Player;

class EnemyState {
public:
    virtual ~EnemyState() = default;
    
    virtual void enter(Enemy* enemy) = 0;
    virtual void update(Enemy* enemy, float dt, Player* player) = 0;
    virtual void exit(Enemy* enemy) = 0;
};

class IdleState : public EnemyState {
public:
    void enter(Enemy* enemy) override;
    void update(Enemy* enemy, float dt, Player* player) override;
    void exit(Enemy* enemy) override;
};

class ChaseState : public EnemyState {
public:
    void enter(Enemy* enemy) override;
    void update(Enemy* enemy, float dt, Player* player) override;
    void exit(Enemy* enemy) override;
};

class ReturnState : public EnemyState {
public:
    void enter(Enemy* enemy) override;
    void update(Enemy* enemy, float dt, Player* player) override;
    void exit(Enemy* enemy) override;
};

}
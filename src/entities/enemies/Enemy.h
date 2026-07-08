#pragma once

#include "../DynamicEntity.h"

namespace Platformer {

enum class EnemyState {
    IDLE,
    CHASE,
    RETURN
};

class Enemy : public DynamicEntity {
protected:
    int health;
    int damage;
    EnemyState currentState;

    float animTimer;
    float animSpeed;
    int currentFrame;
    int numFrames;
    int baseFrameX;
    int baseFrameY;

public:
    Enemy(float x, float y, float w, float h, int hp = 1, int dmg = 1);
    virtual ~Enemy();

    void setAnimation(int frames, float speed, int baseX, int baseY);

    virtual void update(float dt, class Player* player = nullptr) override;
    virtual void render(float lightLevel) override;

    int getHealth() const;
    int getDamage() const;
    EnemyState getState() const;
    
    virtual void takeDamage(int dmg);
};

}

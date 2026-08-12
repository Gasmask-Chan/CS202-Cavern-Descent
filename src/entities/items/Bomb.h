#pragma once

#include "../DynamicEntity.h"

namespace Platformer {

class Bomb : public DynamicEntity {
private:
    float fuseTimer;
    float prevVy;
    float prevVx;

public:
    Bomb(float x, float y, float vx, float vy);
    virtual ~Bomb();

    void update(float dt, class Player* player = nullptr) override;
    void render(float lightLevel) override;
};

}

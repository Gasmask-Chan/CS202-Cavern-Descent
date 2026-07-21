#pragma once
#include "Enemy.h"

namespace Platformer {

class Flame : public Enemy {
public:
    Flame(float x, float y, float vy);
    void update(float dt, Player* player) override;
    void render(float lightLevel) override;
};

}

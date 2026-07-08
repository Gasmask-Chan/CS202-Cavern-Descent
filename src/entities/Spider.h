#pragma once

#include "Enemy.h"

namespace Platformer {

class Spider : public Enemy {
private:
    float jumpTimer;

public:
    Spider(float x, float y, float w, float h);
    ~Spider();

    void update(float dt, class Player* player = nullptr) override;
};

}

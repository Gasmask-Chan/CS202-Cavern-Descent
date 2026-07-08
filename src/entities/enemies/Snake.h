#pragma once

#include "Enemy.h"

namespace Platformer {

class Snake : public Enemy {
private:
    float waitTimer;
    float moveSpeed;
    int direction; // 1 for right, -1 for left

public:
    Snake(float x, float y, float w, float h);
    ~Snake();

    void update(float dt, class Player* player = nullptr) override;

    void handleIdle(float dt, class Player* player) override;
    void handleChase(float dt, class Player* player) override;
    void handleReturn(float dt, class Player* player) override;
};

}

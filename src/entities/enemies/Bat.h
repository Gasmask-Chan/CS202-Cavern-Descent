#pragma once

#include "Enemy.h"

namespace Platformer {

class Bat : public Enemy {
private:
    float startY;

public:
    Bat(float x, float y, float w, float h);
    ~Bat();

    void update(float dt, class Player* player = nullptr) override;

    void handleIdle(float dt, class Player* player) override;
    void handleChase(float dt, class Player* player) override;
    void handleReturn(float dt, class Player* player) override;
};

}

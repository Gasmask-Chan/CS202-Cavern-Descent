#pragma once

#include "Enemy.h"

namespace Platformer {

class NemesisGhost : public Enemy {
public:
    NemesisGhost(float x, float y, float w, float h);
    ~NemesisGhost();

    void update(float dt, class Player* player = nullptr) override;
};

}

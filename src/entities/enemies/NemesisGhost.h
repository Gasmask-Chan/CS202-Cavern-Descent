#pragma once

#include "Enemy.h"

namespace Platformer {

class NemesisGhost : public Enemy {
public:
    NemesisGhost(float x, float y, float w, float h);
    ~NemesisGhost();

    void update(float dt, class Player* player = nullptr) override;

protected:
    void updateSpriteRect() override;

public:
    void handleIdle(float dt, class Player* player) override;
    void handleChase(float dt, class Player* player) override;
    void handleReturn(float dt, class Player* player) override;
};

}

#pragma once

#include "Enemy.h"

namespace Platformer {

class Spike : public Enemy {
private:
    bool _blood;

protected:
    void updateSpriteRect() override;

public:
    Spike(float x, float y, float w, float h);
    ~Spike();

    void update(float dt, class Player* player = nullptr) override;
    
    // Override Enemy logic since Spike doesn't move
    void handleIdle(float dt, class Player* player) override;
    void handleChase(float dt, class Player* player) override;
    void handleReturn(float dt, class Player* player) override;
    
    void takeDamage(int amount) override {} // Spikes are indestructible
    
    void setBlood();
};

}

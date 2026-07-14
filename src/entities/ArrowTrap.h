#pragma once
#include "Trap.h"
#include "DynamicEntity.h"
#include "Item.h"
#include "../player/Player.h"
#include "../core/EventBus.h"
#include "../audio/AudioManager.h"
#include <cmath>

namespace Platformer {

class ArrowTrap : public Trap {
private:
    bool activated = false;
    bool facingRight;

public:
    ArrowTrap(float x, float y, bool facingRight);
    
    void updateTrap(float dt, class Player* player, const std::vector<std::unique_ptr<class DynamicEntity>>& enemies, const std::vector<std::unique_ptr<class Item>>& items) override;
    
    void render(float lightLevel) override;
};

}

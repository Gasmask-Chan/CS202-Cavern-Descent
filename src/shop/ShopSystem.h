#pragma once

#include "ShopItem.h"
#include <raylib.h>
#include <vector>
#include <memory>

namespace Platformer {

class Player;

class ShopSystem {
private:
    std::vector<ShopItem> inventory;
    bool isActive;
    int selectedIndex;

public:
    ShopSystem();
    
    // Generates 3-4 random items with prices scaled by floor
    void initializeFromItems(const std::vector<std::unique_ptr<Item>>& levelItems, int floor);
    
    // Validates gold and purchase status, applies effect, plays SFX
    bool attemptPurchase(Player* player, int index);
    
    // Handles up/down selection and purchase input
    void handleInput(Player* player);
    
    // Draws the semi-transparent overlay in screen-space
    void render(Font font);
    
    // Setters / Getters
    void setPlayerInShop(bool inShop);
    bool isPlayerInShop() const;
};

}

#include "ShopSystem.h"
#include <iostream>
#include "../player/Player.h"
#include "../core/GameManager.h"
#include "../core/EventBus.h"

#include "../entities/items/Item.h"

namespace Platformer {

ShopSystem::ShopSystem() : isActive(false), selectedIndex(0) {
}

void ShopSystem::initializeFromItems(const std::vector<std::unique_ptr<Item>>& levelItems, int floor) {
    inventory.clear();
    selectedIndex = 0;
    
    for (const auto& item : levelItems) {
        if (item && item->isShopItem && !item->isPickedUp()) {
            ShopItem si;
            si.isSold = false;
            si.physicalItem = item.get();
            si.type = item->getType();
            
            float multiplier = (1.0f + (floor - 1) * 0.2f);
            
            if (si.type == ItemType::BOMB_PICKUP) {
                si.name = "Bombs (x3)";
                si.price = 2250 * multiplier;
            } else if (si.type == ItemType::ROPE_PICKUP) {
                si.name = "Ropes (x3)";
                si.price = 2250 * multiplier;
            } else if (si.type == ItemType::CHEST) {
                si.name = "Mystery Chest";
                si.price = 2000 * multiplier;
            } else if (si.type == ItemType::LOOT_PICKUP) {
                si.name = "Shiny Loot";
                si.price = 2000 * multiplier; // Ruby is high value
            } else {
                si.name = "Unknown Item";
                si.price = 2000 * multiplier;
            }
            
            inventory.push_back(si);
        }
    }
}

bool ShopSystem::attemptPurchase(Player* player, int index) {
    if (index < 0 || index >= (int)inventory.size()) return false;
    
    ShopItem& item = inventory[index];
    if (item.isSold) {
        return false;
    }
    
    if (player->getGold() >= item.price) {
        bool activated = true;
        if (item.physicalItem) {
            item.physicalItem->isShopItem = false;
            activated = item.physicalItem->activate(player);
        }
        
        if (activated) {
            player->spendGold(item.price);
            item.isSold = true;
            return true;
        } else {
            // Restore shop item status if activation failed
            if (item.physicalItem) {
                item.physicalItem->isShopItem = true;
            }
            return false;
        }
    }
    
    return false;
}

void ShopSystem::handleInput(Player* player) {
    if (!isActive || inventory.empty()) return;
    
    if (IsKeyPressed(KEY_UP)) {
        selectedIndex--;
        if (selectedIndex < 0) selectedIndex = (int)inventory.size() - 1;
    } else if (IsKeyPressed(KEY_DOWN)) {
        selectedIndex++;
        if (selectedIndex >= (int)inventory.size()) selectedIndex = 0;
    }
    
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_Z)) {
        attemptPurchase(player, selectedIndex);
    }
}

void ShopSystem::render(Font font) {
    if (!isActive) return;
    
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    
    int panelW = 500;
    int panelH = 60 + 50 * inventory.size();
    int panelX = (screenW - panelW) / 2;
    int panelY = (screenH - panelH) / 2;
    
    DrawRectangle(panelX, panelY, panelW, panelH, ColorAlpha(BLACK, 0.8f));
    DrawRectangleLines(panelX, panelY, panelW, panelH, GOLD);
    
    DrawTextEx(font, "SHOP", Vector2{(float)panelX + 210, (float)panelY + 10}, 30, 2, GOLD);
    
    for (size_t i = 0; i < inventory.size(); i++) {
        const ShopItem& item = inventory[i];
        
        Color textColor = item.isSold ? DARKGRAY : WHITE;
        Color priceColor = item.isSold ? DARKGRAY : GOLD;
        
        float startY = panelY + 60 + i * 50;
        
        if (!item.isSold && (int)i == selectedIndex) {
            DrawRectangle(panelX + 10, startY - 5, panelW - 20, 40, ColorAlpha(DARKGRAY, 0.5f));
            DrawTextEx(font, ">", Vector2{(float)panelX + 20, startY}, 20, 2, WHITE);
        }
        
        DrawTextEx(font, item.name.c_str(), Vector2{(float)panelX + 50, startY}, 20, 2, textColor);
        
        if (item.isSold) {
            DrawTextEx(font, "SOLD", Vector2{(float)panelX + 380, startY}, 20, 2, DARKGRAY);
        } else {
            std::string priceStr = "$" + std::to_string(item.price);
            DrawTextEx(font, priceStr.c_str(), Vector2{(float)panelX + 380, startY}, 20, 2, priceColor);
        }
    }
}

void ShopSystem::setPlayerInShop(bool inShop) {
    isActive = inShop;
}

bool ShopSystem::isPlayerInShop() const {
    return isActive;
}

}

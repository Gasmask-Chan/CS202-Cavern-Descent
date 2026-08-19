#include "Item.h"
#include "../../player/Player.h"
#include "../../core/EventBus.h"
#include "../../audio/AudioManager.h"
#include <cmath>

namespace Platformer {

Item::Item(float x, float y, float w, float h, ItemType type)
    : DynamicEntity(x, y, w, h), type(type), isCollected(false), prevVy(0.0f), isHeld(false) {}

void Item::update(float dt, Player* player) {
    if (isHeld && player) {
        // Stick to player's center/hands
        this->x = player->getX() + (player->getAABB().width - this->width) / 2.0f;
        this->y = player->getY() + player->getAABB().height / 2.0f - this->height;
        this->vx = 0;
        this->vy = 0;
    } else if (isEmbedded) {
        this->vx = 0;
        this->vy = 0;
        // Skip gravity. isEmbedded is unset by GameState when the block is destroyed.
    } else {
        prevVy = this->vy;
        applyGravity(dt);
        if (isGrounded) {
            vx *= 0.8f; // friction
            if (std::abs(vx) < 5.0f) vx = 0;
        }
        move(vx * dt, vy * dt);
        DynamicEntity::update(dt, player);
    }
}

bool Item::activate(Player* player) {
    collect();
    return true;
}

void Item::render(float lightLevel) {
    if (isAlive() && !isCollected) {
        if (!isEmbedded) {
            DynamicEntity::render(lightLevel);
        }
    }
}

void Item::collect() {
    isCollected = true;
    destroy();
}

bool Item::isPickedUp() {
    return isCollected;
}

ItemType Item::getType() {
    return type;
}

// LootPickup
LootPickup::LootPickup(float x, float y, float w, float h, int val)
    : Item(x, y, w, h, ItemType::LOOT_PICKUP), value(val) {}

bool LootPickup::activate(Player* player) {
    if (value > 100) {
        AudioManager::getInstance()->playSFX("xgem");
    } else {
        AudioManager::getInstance()->playSFX("xcoin");
    }
    player->collectGold(value);
    collect();
    return true;
}

// Chest
Chest::Chest(float x, float y, float w, float h)
    : Item(x, y, w, h, ItemType::CHEST), isOpened(false) {}

void Chest::update(float dt, Player* player) {
    Item::update(dt, player);
}

bool Chest::activate(Player* player) {
    if (!isOpened && !isShopItem) {
        // Open Chest
        isOpened = true;
        AudioManager::getInstance()->playSFX("xchestopen");
        
        // Change sprite to open chest (Row 0, Col 3 -> {48, 0, 16, 16})
        srcRect.x = 48.0f;
        
        // Spawn 4 rubies bursting out
        for (int i = 0; i < 4; i++) {
            EventData data;
            char lootType = (GetRandomValue(0, 1) == 0) ? 'R' : 'G'; // R=Ruby Big, G=Ruby Small
            data.amount = lootType;
            
            if (lootType == 'G') {
                // Gold is 32x32, same as chest, so spawn exactly at chest bounds to prevent clipping
                data.worldX = this->x;
                data.worldY = this->y;
            } else {
                // Rubies are 16x16, center them inside the 32x32 chest
                data.worldX = this->x + 8.0f;
                data.worldY = this->y + 8.0f;
            }
            
            // Set burst velocity with high variance to pop out of the chest significantly
            data.vy = -3.5f * 150.0f + (float)GetRandomValue(-50, 50); 
            float baseVx = (GetRandomValue(0, 1) == 0 ? -0.8f : 0.8f) * 150.0f;
            data.vx = baseVx + (float)GetRandomValue(-40, 40);
            
            EventBus::getInstance()->publish(EventType::EVENT_SPAWN_ITEM, data);
        }
        return true;
    }
    return false;
}

// BombPickup
BombPickup::BombPickup(float x, float y, float w, float h, int amount)
    : Item(x, y, w, h, ItemType::BOMB_PICKUP), amount(amount) {}

bool BombPickup::activate(Player* player) {
    AudioManager::getInstance()->playSFX("xpickup");
    player->addBomb(amount);
    collect();
    return true;
}

// RopePickup
RopePickup::RopePickup(float x, float y, float w, float h, int amount)
    : Item(x, y, w, h, ItemType::ROPE_PICKUP), amount(amount) {}

bool RopePickup::activate(Player* player) {
    AudioManager::getInstance()->playSFX("xpickup");
    player->addRope(amount);
    collect();
    return true;
}

}

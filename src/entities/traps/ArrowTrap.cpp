#include "ArrowTrap.h"
#include "../../level/TileMap.h"
#include "../enemies/Enemy.h"
#include "../enemies/NemesisGhost.h"
#include "../enemies/Spike.h"
#include "../enemies/Flame.h"
#include "../items/Bomb.h"
#include "../../audio/AudioManager.h"

namespace Platformer {

ArrowTrap::ArrowTrap(float x, float y, bool facingRight) 
    : Trap(x, y, 32.0f, 32.0f, 0), facingRight(facingRight) {
    // We set damage to 0 because the trap itself doesn't hurt you anymore; the arrow does.
    // The hitbox is 32x32 exactly over the tile.
}

void ArrowTrap::updateTrap(float dt, Player* player, const std::vector<std::unique_ptr<DynamicEntity>>& enemies, const std::vector<std::unique_ptr<Item>>& items, TileMap* tileMap) {
    if (activated) return;

    auto checkLOS = [&](float objX, float objY, float objW, float objH) {
        
        // Check if on same row (y matches). The object's Y bounds must overlap with the trap's Y bounds.
        if (!(objY + objH > y && objY < y + height)) return false;
        
        float dist = objX - x;
        float maxDist = 7.0f * 32.0f; // 7 tiles
        if (facingRight) {
            if (dist > 0 && dist <= maxDist) {
                if (tileMap) {
                    int startX = static_cast<int>((x + width) / 32.0f);
                    int endX = static_cast<int>(objX / 32.0f);
                    if (startX > endX) std::swap(startX, endX);
                    int ty = static_cast<int>((y + height / 2.0f) / 32.0f);
                    for (int tx = startX; tx <= endX; ++tx) {
                        if (tileMap->isSolid(tx, ty)) return false;
                    }
                }
                return true;
            }
        } else {
            if (dist < 0 && -dist <= maxDist) {
                if (tileMap) {
                    int startX = static_cast<int>((objX + objW) / 32.0f);
                    int endX = static_cast<int>(x / 32.0f) - 1;
                    if (startX > endX) std::swap(startX, endX);
                    int ty = static_cast<int>((y + height / 2.0f) / 32.0f);
                    for (int tx = startX; tx <= endX; ++tx) {
                        if (tileMap->isSolid(tx, ty)) return false;
                    }
                }
                return true;
            }
        }
        return false;
    };
    
    bool trigger = false;
    if (player && player->isAlive() && checkLOS(player->getX(), player->getY(), player->getAABB().width, player->getAABB().height)) {
        trigger = true;
    }
    if (!trigger) {
        for (const auto& entity : enemies) {
            if (entity && entity->isAlive() && 
               (std::abs(entity->getVelocityX()) > 10.0f || std::abs(entity->getVelocityY()) > 10.0f)) {
                
                bool validTrigger = false;
                if (auto* enemy = dynamic_cast<Enemy*>(entity.get())) {
                    if (!dynamic_cast<NemesisGhost*>(enemy) && !dynamic_cast<Spike*>(enemy) && !dynamic_cast<Flame*>(enemy)) {
                        validTrigger = true;
                    }
                } else if (dynamic_cast<Bomb*>(entity.get())) {
                    validTrigger = true;
                }

                if (validTrigger && checkLOS(entity->getX(), entity->getY(), entity->getAABB().width, entity->getAABB().height)) {
                    trigger = true;
                    break;
                }
            }
        }
    }
    if (!trigger) {
        for (const auto& item : items) {
            if (item && !item->isPickedUp() && 
               (std::abs(item->getVelocityX()) > 10.0f || std::abs(item->getVelocityY()) > 10.0f) && 
               checkLOS(item->getX(), item->getY(), item->getAABB().width, item->getAABB().height)) {
                trigger = true;
                break;
            }
        }
    }
    
    if (trigger) {
        activated = true;
        
        EventData data;
        // Spawn slightly outside the trap so it doesn't get stuck inside its own wall tile
        data.worldX = facingRight ? x + width + 2.0f : x - 18.0f;
        data.worldY = y + 8.0f; // Center arrow vertically (arrow is 16x16, trap is 32x32, so 8+16 = 24. Actually, middle is y+8)
        data.vx = facingRight ? 800.0f : -800.0f;
        data.vy = 0.0f;
        
        EventBus::getInstance()->publish(EventType::EVENT_SPAWN_ARROW, data);
        AudioManager::getInstance()->playSFX("xarrowtrap");
    }
}

void ArrowTrap::render(float lightLevel) {
    // Completely invisible! Render nothing.
}

}

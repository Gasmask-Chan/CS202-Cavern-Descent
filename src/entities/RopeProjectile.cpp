#include "RopeProjectile.h"
#include "../level/TileMap.h"
#include "../audio/AudioManager.h"
#include <cmath>

namespace Platformer {

RopeProjectile::RopeProjectile(float x, float y, float vy, TileMap* map)
    : DynamicEntity(x, y, 8.0f, 8.0f), startY(y), isUnfurling(false), 
      unfurlTimer(0.0f), currentLength(0), maxLength(6), tileMap(map) {
    this->vx = 0.0f;
    this->vy = vy;
    this->gravity = 0.0f;
    this->passesThroughWalls = false;
}

void RopeProjectile::update(float dt, Player* player) {
    if (!isUnfurling) {
        // Still flying up
        float distTraveled = startY - y;
        
        // Stop if we hit a ceiling (vy became 0 from PhysicsSystem) or traveled max distance (6 tiles = 192 pixels)
        if (vy == 0.0f || distTraveled >= maxLength * 32.0f) {
            isUnfurling = true;
            this->vy = 0.0f;
            this->vx = 0.0f;
            
            // Snap to grid
            int tx = static_cast<int>(x / 32.0f);
            int ty = static_cast<int>(y / 32.0f);
            x = tx * 32.0f + 16.0f - width / 2.0f;
            y = ty * 32.0f;
            
            AudioManager::getInstance()->playSFX("hit"); // Fallback sound for rope latching
        } else {
            // Apply upward movement
            move(0.0f, vy * dt);
        }
    } else {
        // Unfurling phase (places tiles downward over time)
        unfurlTimer += dt;
        if (unfurlTimer >= 0.05f) { // Place one tile every 0.05 seconds for visual effect
            unfurlTimer = 0.0f;
            
            int tx = static_cast<int>((x + width / 2.0f) / 32.0f);
            int ty = static_cast<int>(y / 32.0f) + currentLength;
            
            if (tileMap && tileMap->isInBounds(tx, ty)) {
                // If it hits solid floor, stop unfurling
                if (tileMap->isSolid(tx, ty)) {
                    destroy();
                    return;
                }
                
                // Place rope node (skip if there's already a rope there to prevent overwriting other important tiles unnecessarily, though Spelunky does overwrite air)
                if (tileMap->getTile(tx, ty) == TileType::NOTHING || tileMap->getTile(tx, ty) == TileType::ROPE_NODE) {
                    tileMap->setTile(tx, ty, TileType::ROPE_NODE);
                }
                
                currentLength++;
                if (currentLength >= maxLength) {
                    destroy();
                }
            } else {
                destroy();
            }
        }
    }
}

void RopeProjectile::render(float lightLevel) {
    if (!isUnfurling) {
        // Draw the rope head flying up
        Color tint = { 
            (unsigned char)(255 * lightLevel), 
            (unsigned char)(255 * lightLevel), 
            (unsigned char)(255 * lightLevel), 
            255 
        };
        // Use a generic rectangle or load a small part of the rope texture if we wanted
        DrawRectangleRec(Rectangle{x, y, width, height}, tint);
    }
}

}

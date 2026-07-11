#include "PhysicsSystem.h"
#include <cmath>
#include <algorithm>
#include <cstdio>

namespace Platformer {

PhysicsSystem::PhysicsSystem(TileMap* map) : tileMap(map) {}

bool PhysicsSystem::checkAABBOverlap(Rectangle a, Rectangle b) {
    return (a.x < b.x + b.width &&
            a.x + a.width > b.x &&
            a.y < b.y + b.height &&
            a.y + a.height > b.y);
}

void PhysicsSystem::resolveEntityTileCollision(DynamicEntity* e) {
    if (e->passesThroughWalls) return;
    
    Rectangle aabb = e->getAABB();
    
    int startX = static_cast<int>(std::floor(aabb.x / tileMap->getTileSize()));
    int endX = static_cast<int>(std::floor((aabb.x + aabb.width) / tileMap->getTileSize()));
    int startY = static_cast<int>(std::floor(aabb.y / tileMap->getTileSize()));
    int endY = static_cast<int>(std::floor((aabb.y + aabb.height) / tileMap->getTileSize()));

    e->isGrounded = false; 

    for (int y = startY; y <= endY; ++y) {
        for (int x = startX; x <= endX; ++x) {
            if (tileMap->isSolid(x, y)) {
                Rectangle tileRect = {
                    static_cast<float>(x * tileMap->getTileSize()),
                    static_cast<float>(y * tileMap->getTileSize()),
                    static_cast<float>(tileMap->getTileSize()),
                    static_cast<float>(tileMap->getTileSize())
                };

                if (checkAABBOverlap(aabb, tileRect)) {
                    float penLeft = (aabb.x + aabb.width) - tileRect.x;
                    float penRight = (tileRect.x + tileRect.width) - aabb.x;
                    float penTop = (aabb.y + aabb.height) - tileRect.y;
                    float penBottom = (tileRect.y + tileRect.height) - aabb.y;

                    float minPen = std::min({penLeft, penRight, penTop, penBottom});

                    // If penTop is extremely small, it means the entity is just sliding along the floor.
                    // Prioritize floor collision over wall collision to prevent getting stuck on flat tiles.
                    if (minPen == penTop || (penTop < 2.0f && e->vy >= 0)) {
                        e->move(0, -penTop);
                        e->isGrounded = true;
                        if (e->vy > 0) e->vy = 0;
                    } else if (minPen == penBottom) {
                        e->move(0, penBottom);
                        if (e->vy < 0) e->vy = 0;
                    } else if (minPen == penLeft) {
                        e->move(-penLeft, 0);
                        e->vx = 0;
                    } else if (minPen == penRight) {
                        e->move(penRight, 0);
                        e->vx = 0;
                    }
                    
                    aabb = e->getAABB(); 
                }
            }
        }
    }
}

CollisionResult PhysicsSystem::sweepAABB(DynamicEntity* e, float dt) {
    CollisionResult result;
    result.collided = false;
    result.contactTime = 1.0f;
    result.contactNormal = {0.0f, 0.0f};
    result.contactPoint = {0.0f, 0.0f};
    
    if (e->passesThroughWalls) return result;
    
    float length = std::sqrt(e->vx * e->vx + e->vy * e->vy) * dt;
    if (length <= 0.0f) return result;
    
    int steps = static_cast<int>(std::ceil(length / 4.0f));
    if (steps == 0) steps = 1;

    float stepX = (e->vx * dt) / steps;
    float stepY = (e->vy * dt) / steps;
    
    Rectangle aabb = e->getAABB();
    for (int i = 1; i <= steps; ++i) {
        aabb.x += stepX;
        aabb.y += stepY;
        
        int startX = static_cast<int>(std::floor(aabb.x / tileMap->getTileSize()));
        int endX = static_cast<int>(std::floor((aabb.x + aabb.width) / tileMap->getTileSize()));
        int startY = static_cast<int>(std::floor(aabb.y / tileMap->getTileSize()));
        int endY = static_cast<int>(std::floor((aabb.y + aabb.height) / tileMap->getTileSize()));
        
        for (int y = startY; y <= endY; ++y) {
            for (int x = startX; x <= endX; ++x) {
                if (tileMap->isSolid(x, y)) {
                    result.collided = true;
                    result.contactTime = static_cast<float>(i) / steps;
                    if (std::abs(e->vx) > std::abs(e->vy)) {
                        result.contactNormal.x = (e->vx > 0) ? -1.0f : 1.0f;
                    } else {
                        result.contactNormal.y = (e->vy > 0) ? -1.0f : 1.0f;
                    }
                    result.contactPoint = {aabb.x + aabb.width / 2.0f, aabb.y + aabb.height / 2.0f};
                    return result;
                }
            }
        }
    }
    
    return result;
}

}

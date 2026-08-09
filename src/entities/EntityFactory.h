#pragma once

#include <memory>
#include <string>
#include "DynamicEntity.h"
#include "items/Item.h"
#include "traps/Trap.h"

namespace Platformer {

/**
 * @brief Factory class to parse character codes from room templates and instantiate
 * the appropriate concrete Entity subclasses.
 */
class EntityFactory {
public:
    static Texture2D getTexture(const std::string& path);
    static void preloadTextures();

    /**
     * @brief Creates an enemy entity based on the character code.
     * Returns nullptr if the code does not correspond to an enemy.
     */
    static std::unique_ptr<DynamicEntity> createEnemy(char code, float x, float y);

    /**
     * @brief Creates a ghost entity.
     */
    static std::unique_ptr<DynamicEntity> createGhost(float x, float y);

    /**
     * @brief Creates an item entity based on the character code.
     * Returns nullptr if the code does not correspond to an item.
     */
    static std::unique_ptr<Item> createItem(char code, float x, float y);

    /**
     * @brief Creates a trap entity based on the character code.
     * Returns nullptr if the code does not correspond to a trap.
     */
    static std::unique_ptr<Trap> createTrap(char code, float x, float y);
    
    static std::unique_ptr<class Arrow> createArrow(float x, float y, float vx);
    
    static std::unique_ptr<class Explosion> createExplosion(float x, float y);
    static std::unique_ptr<class Particle> createBloodParticle(float x, float y);
};

}

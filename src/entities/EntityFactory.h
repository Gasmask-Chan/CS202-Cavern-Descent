#pragma once

#include <memory>
#include "DynamicEntity.h"
#include "Item.h"
#include "Trap.h"

namespace Platformer {

/**
 * @brief Factory class to parse character codes from room templates and instantiate
 * the appropriate concrete Entity subclasses.
 */
class EntityFactory {
public:
    /**
     * @brief Creates an enemy entity based on the character code.
     * Returns nullptr if the code does not correspond to an enemy.
     */
    static std::unique_ptr<DynamicEntity> createEnemy(char code, float x, float y);

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
};

}

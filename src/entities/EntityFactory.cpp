#include "EntityFactory.h"

namespace Platformer {

std::unique_ptr<DynamicEntity> EntityFactory::createEnemy(char code, float x, float y) {
    switch (code) {
        case 'E': // Generic/Random Enemy
        case 'B': // Bat
        case 'S': // Snake
        case 'P': // Spider
            // TODO: Return concrete Enemy instances once they are implemented
            return nullptr;
        default:
            return nullptr;
    }
}

std::unique_ptr<Item> EntityFactory::createItem(char code, float x, float y) {
    switch (code) {
        case 'T': // Treasure
        case 'H': // Health Crate
        case 'X': // Bomb Pickup
        case 'R': // Rope Pickup
            // TODO: Return concrete Item instances once they are implemented
            return nullptr;
        default:
            return nullptr;
    }
}

std::unique_ptr<Trap> EntityFactory::createTrap(char code, float x, float y) {
    switch (code) {
        case '^': // Spike Trap
        case '>': // Arrow Trap (Right)
        case '<': // Arrow Trap (Left)
            // TODO: Return concrete Trap instances once they are implemented
            return nullptr;
        default:
            return nullptr;
    }
}

}

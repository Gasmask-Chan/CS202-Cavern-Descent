#include "MovementStrategy.h"

namespace Platformer {

float ExplorerStrategy::getMoveSpeed() { return 175.0f; }
float ExplorerStrategy::getJumpForce() { return 450.0f; }
int ExplorerStrategy::getMaxHealth() { return 4; }
float ExplorerStrategy::getGravityScale() { return 1.0f; }

float NinjaStrategy::getMoveSpeed() { return 215.0f; }
float NinjaStrategy::getJumpForce() { return 480.0f; }
int NinjaStrategy::getMaxHealth() { return 2; }
float NinjaStrategy::getGravityScale() { return 0.95f; }

float TankStrategy::getMoveSpeed() { return 135.0f; }
float TankStrategy::getJumpForce() { return 400.0f; }
int TankStrategy::getMaxHealth() { return 6; }
float TankStrategy::getGravityScale() { return 1.15f; }

}

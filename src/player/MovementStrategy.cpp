#include "MovementStrategy.h"

namespace Platformer {

float ExplorerStrategy::getMoveSpeed() { return 150.0f; }
float ExplorerStrategy::getJumpForce() { return 370.0f; }
int ExplorerStrategy::getMaxHealth() { return 5; }
float ExplorerStrategy::getGravityScale() { return 1.0f; }

float NinjaStrategy::getMoveSpeed() { return 170.0f; }
float NinjaStrategy::getJumpForce() { return 420.0f; }
int NinjaStrategy::getMaxHealth() { return 4; }
float NinjaStrategy::getGravityScale() { return 0.95f; }

float TankStrategy::getMoveSpeed() { return 110.0f; }
float TankStrategy::getJumpForce() { return 360.0f; }
int TankStrategy::getMaxHealth() { return 7; }
float TankStrategy::getGravityScale() { return 1.15f; }

}

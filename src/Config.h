#pragma once

/**
 * @brief This file should be included if Raylib library is needed
 *
 */

#include "raylib.h"
#include "raymath.h"

namespace Platformer {

// Raylib `Vector2` but with integer coordinates
struct Vector2i {
  int x;
  int y;

  // Automatically casting to Raylib `Vector2`
  operator Vector2() const {
    return Vector2{static_cast<float>(x), static_cast<float>(y)};
  }
};

} // namespace Platformer
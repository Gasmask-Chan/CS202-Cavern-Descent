#include "MenuBackground.h"
#include "raylib.h"
#include "../entities/EntityFactory.h"

namespace Platformer {

void MenuBackground::render() {
  Texture2D bgTex = EntityFactory::getTexture("assets/tilemaps/gfx_cavebg.png");
  if (bgTex.id != 0) {
      Rectangle bgSrc = { 0.0f, 336.0f, 32.0f, 128.0f };
      float scale = 4.0f; // Scale up for better visibility
      for (float x = 0; x < 1280.0f; x += 32.0f * scale) {
          for (float y = 0; y < 720.0f; y += 128.0f * scale) {
              DrawTexturePro(bgTex, bgSrc, {x, y, 32.0f * scale, 128.0f * scale}, {0,0}, 0.0f, WHITE);
          }
      }
  }
}

} // namespace Platformer

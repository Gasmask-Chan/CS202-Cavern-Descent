#include "src/level/LevelGenerator.h"
#include <iostream>

using namespace Platformer;

int main() {
  int tileSize = 24;
  int screenWidth = 960;
  int screenHeight = 960;

  InitWindow(screenWidth, screenHeight,
             "Cavern Descent - Level Generator Visualizer");
  SetTargetFPS(60);

  LevelGenerator gen;
  std::cout << "Starting Visual Generation Test..." << std::endl;

  // Generate the first level
  GeneratedLevel lvl = gen.generate(1, ZoneType::CAVE);

  while (!WindowShouldClose()) {
    // Press SPACE to generate a new random level
    if (IsKeyPressed(KEY_SPACE)) {
      lvl = gen.generate(1, ZoneType::CAVE);
    }

    BeginDrawing();
    ClearBackground(BLACK);

    // Draw the map using the real render method to see textures
    if (lvl.tileMap) {
      Camera2D dummyCam = {0};
      std::vector<std::vector<float>> dummyLightMap;
      lvl.tileMap->render(dummyCam, dummyLightMap);
    }

    // Draw Player Spawn
    DrawRectangle(lvl.playerSpawn.x / 32.0f * tileSize,
                  lvl.playerSpawn.y / 32.0f * tileSize, tileSize, tileSize,
                  BLUE);

    DrawText("Press SPACE to generate a new level", 10, 10, 20, RAYWHITE);
    DrawText("Gray=Wall, Brown=Cracked, Green=Exit, Blue=Spawn", 10, 35, 20,
             RAYWHITE);

    EndDrawing();
  }

  CloseWindow();
  return 0;
}

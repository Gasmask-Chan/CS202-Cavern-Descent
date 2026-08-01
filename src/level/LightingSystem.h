#pragma once

#include <vector>
#include "TileMap.h"

namespace Platformer {

struct LightSource {
    float fx;
    float fy;
    Vector3 color;
    float radius;
};

class LightingSystem {
private:
    std::vector<std::vector<Vector3>> lightMap;
    std::vector<LightSource> sources;
    // Ambient light level
    Vector3 ambientLight = {0.15f, 0.15f, 0.25f};
    int width;
    int height;

    // Shadowcasting algorithm methods
    void castLight(TileMap* map, int cx, int cy, float fx, float fy, int row, float startSlope, float endSlope, float radius, int xx, int xy, int yx, int yy, Vector3 color);

public:
    LightingSystem(int mapWidth, int mapHeight);

    void setAmbientLight(Vector3 level);
    
    // Clears dynamic sources and resets light map to ambient level
    void clearLights();
    
    // Queues a light source for the current frame
    void addLight(float fx, float fy, Vector3 color, float radius);
    
    // Computes FOV for all sources and updates the lightMap
    void update(TileMap* map);
    
    // Returns the calculated light intensities for rendering
    const std::vector<std::vector<Vector3>>& getLightMap() const;
};

}

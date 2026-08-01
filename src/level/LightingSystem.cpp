#include "LightingSystem.h"
#include <algorithm>
#include <cmath>
#include "raymath.h"

namespace Platformer {

LightingSystem::LightingSystem(int mapWidth, int mapHeight) 
    : ambientLight({0.15f, 0.15f, 0.25f}), width(mapWidth), height(mapHeight) {
    lightMap.resize(height, std::vector<Vector3>(width, ambientLight));
}

void LightingSystem::setAmbientLight(Vector3 level) {
    ambientLight = level;
}

void LightingSystem::clearLights() {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            lightMap[y][x] = ambientLight;
        }
    }
    sources.clear();
}

void LightingSystem::addLight(float fx, float fy, Vector3 color, float radius) {
    sources.push_back({fx, fy, color, radius});
}

void LightingSystem::update(TileMap* map) {
    for (const auto& src : sources) {
        int cx = static_cast<int>(std::floor(src.fx));
        int cy = static_cast<int>(std::floor(src.fy));
        
        if (cx >= 0 && cx < width && cy >= 0 && cy < height) {
            // Light the origin tile
            float srcLum = (src.color.x + src.color.y + src.color.z) / 3.0f;
            float curLum = (lightMap[cy][cx].x + lightMap[cy][cx].y + lightMap[cy][cx].z) / 3.0f;
            if (srcLum > curLum) {
                lightMap[cy][cx] = src.color;
            }
            
            // If the light source itself is inside a wall, do not cast light outward to prevent bleeding.
            if (map->isOpaque(cx, cy)) {
                continue;
            }
            
            // Cast light into all 8 octants
            castLight(map, cx, cy, src.fx, src.fy, 1, 1.0f, 0.0f, src.radius,  1,  0,  0,  1, src.color);
            castLight(map, cx, cy, src.fx, src.fy, 1, 1.0f, 0.0f, src.radius,  1,  0,  0, -1, src.color);
            castLight(map, cx, cy, src.fx, src.fy, 1, 1.0f, 0.0f, src.radius, -1,  0,  0,  1, src.color);
            castLight(map, cx, cy, src.fx, src.fy, 1, 1.0f, 0.0f, src.radius, -1,  0,  0, -1, src.color);
            castLight(map, cx, cy, src.fx, src.fy, 1, 1.0f, 0.0f, src.radius,  0,  1,  1,  0, src.color);
            castLight(map, cx, cy, src.fx, src.fy, 1, 1.0f, 0.0f, src.radius,  0,  1, -1,  0, src.color);
            castLight(map, cx, cy, src.fx, src.fy, 1, 1.0f, 0.0f, src.radius,  0, -1,  1,  0, src.color);
            castLight(map, cx, cy, src.fx, src.fy, 1, 1.0f, 0.0f, src.radius,  0, -1, -1,  0, src.color);
        }
    }
}

void LightingSystem::castLight(TileMap* map, int cx, int cy, float fx, float fy, int row, float startSlope, float endSlope, float radius, int xx, int xy, int yx, int yy, Vector3 color) {
    if (startSlope <= endSlope) return;
    float nextStartSlope = startSlope;
    for (int i = row; i <= radius; i++) {
        bool blocked = false;
        for (int dx = -i; dx <= 0; dx++) {
            int dy = -i;
            int sax = dx * xx + dy * xy;
            int say = dx * yx + dy * yy;
            int ax = cx + sax;
            int ay = cy + say;
            
            if (ax < 0 || ay < 0 || ax >= width || ay >= height) continue;

            // Compute FOV slopes from the center of the origin tile (cx, cy) 
            // rather than the exact sub-tile position (fx, fy).
            // This prevents octant boundaries from sweeping across tiles and 
            // artificially darkening them via the MAX blend when split.
            float diff_x = (float)(ax - cx);
            float diff_y = (float)(ay - cy);
            
            float octant_dx = diff_x * xx + diff_y * yx;
            float octant_dy = diff_x * xy + diff_y * yy;

            auto get_slope = [](float x, float y) {
                if (std::abs(y) < 0.0001f) y = -0.0001f;
                return x / y;
            };

            float s1 = get_slope(octant_dx - 0.5f, octant_dy - 0.5f);
            float s2 = get_slope(octant_dx + 0.5f, octant_dy - 0.5f);
            float s3 = get_slope(octant_dx - 0.5f, octant_dy + 0.5f);
            float s4 = get_slope(octant_dx + 0.5f, octant_dy + 0.5f);

            float l_slope = std::max({s1, s2, s3, s4});
            float r_slope = std::min({s1, s2, s3, s4});

            if (startSlope < r_slope) continue;
            else if (endSlope > l_slope) break;

            float true_dx = (ax + 0.5f) - fx;
            float true_dy = (ay + 0.5f) - fy;
            float distance = std::sqrt(true_dx * true_dx + true_dy * true_dy);
            
            if (distance <= radius) {
                // Inverse-square falloff
                float falloff = 1.0f - (distance / radius);
                // Boost the falloff slightly so it's less linear and more spherical
                falloff = std::pow(falloff, 1.5f); 
                Vector3 currentIntensity = { color.x * falloff, color.y * falloff, color.z * falloff };
                
                float overlap_top = std::min(startSlope, l_slope);
                float overlap_bottom = std::max(endSlope, r_slope);
                
                float visibility = 0.0f;
                if (overlap_top > overlap_bottom) {
                    float cell_width = l_slope - r_slope;
                    if (cell_width > 0.0001f) {
                        visibility = (overlap_top - overlap_bottom) / cell_width;
                    } else {
                        visibility = 1.0f;
                    }
                }
                
                if (visibility > 0.0f) {
                    Vector3 finalIntensity = {
                        ambientLight.x + (currentIntensity.x - ambientLight.x) * visibility,
                        ambientLight.y + (currentIntensity.y - ambientLight.y) * visibility,
                        ambientLight.z + (currentIntensity.z - ambientLight.z) * visibility
                    };
                    lightMap[ay][ax].x = std::max(lightMap[ay][ax].x, finalIntensity.x);
                    lightMap[ay][ax].y = std::max(lightMap[ay][ax].y, finalIntensity.y);
                    lightMap[ay][ax].z = std::max(lightMap[ay][ax].z, finalIntensity.z);
                }
            }

            bool isSolid = map->isOpaque(ax, ay);
            if (blocked) {
                if (isSolid) {
                    nextStartSlope = r_slope;
                } else {
                    blocked = false;
                    startSlope = nextStartSlope;
                }
            } else {
                if (isSolid && i < radius) {
                    blocked = true;
                    castLight(map, cx, cy, fx, fy, i + 1, startSlope, l_slope, radius, xx, xy, yx, yy, color);
                    nextStartSlope = r_slope;
                }
            }
        }
        if (blocked) break;
    }
}

const std::vector<std::vector<Vector3>>& LightingSystem::getLightMap() const {
    return lightMap;
}

}

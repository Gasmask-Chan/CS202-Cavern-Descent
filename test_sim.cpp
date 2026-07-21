#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>

enum class LiquidType { NONE, WATER, LAVA };

int GetRandomValue(int min, int max) {
    return min + (rand() % (max - min + 1));
}

int main() {
    srand(time(NULL));
    int width = 50;
    int height = 50;
    std::vector<std::vector<bool>> hasLiquid(height, std::vector<bool>(width, false));
    std::vector<std::vector<LiquidType>> typeGrid(height, std::vector<LiquidType>(width, LiquidType::NONE));
    std::vector<std::vector<bool>> isSpurtBlock(height, std::vector<bool>(width, false));
    std::vector<std::vector<float>> spurtTimer(height, std::vector<float>(width, 0.0f));

    auto addLiquid = [&](int gx, int gy, LiquidType type) {
        hasLiquid[gy][gx] = true;
        typeGrid[gy][gx] = type;
        if (type == LiquidType::LAVA && GetRandomValue(1, 4) == 1) {
            isSpurtBlock[gy][gx] = true;
            spurtTimer[gy][gx] = (float)GetRandomValue(60, 180) / 60.0f;
        }
    };

    // instantiateLakeRoom pattern
    for (int y = 3; y <= 7; ++y) {
        for (int x = 2; x <= 7; ++x) {
            addLiquid(x, y, LiquidType::LAVA);
        }
    }

    int count = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (hasLiquid[y][x] && typeGrid[y][x] == LiquidType::LAVA && isSpurtBlock[y][x]) {
                if (y == 0 || !hasLiquid[y - 1][x]) {
                    count++;
                }
            }
        }
    }

    std::cout << "Surface spurt blocks: " << count << std::endl;
    return 0;
}

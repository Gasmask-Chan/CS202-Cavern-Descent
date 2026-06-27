#pragma once

#include <string>
#include <vector>

namespace Platformer {

enum class RoomRole {
    UNKNOWN,
    TYPE_0, // Side room
    TYPE_1, // Left/Right pass
    TYPE_2, // Left/Right/Bottom pass (Drop room)
    TYPE_3, // Left/Right/Top pass (Landing room)
    TYPE_2_DROP_THROUGH // Left/Right/Top/Bottom pass
};

class RoomTemplate {
private:
    std::string filePath;
    RoomRole role;
    std::vector<std::vector<char>> grid;

public:
    RoomTemplate(std::string path);

    /**
     * @brief Reads the file and populates the 2D char grid.
     * @return true if successful, false otherwise.
     */
    bool load();

    std::vector<std::vector<char>> getGrid() const;
    
    RoomRole getRole() const;
};

}

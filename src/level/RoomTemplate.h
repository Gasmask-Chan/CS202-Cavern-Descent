#pragma once

#include <string>
#include <vector>

namespace Platformer {

enum class RoomRole {
    PATH,
    SIDE,
    SHOP,
    TREASURE,
    UNKNOWN
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

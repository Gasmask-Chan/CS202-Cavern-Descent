#include "RoomTemplate.h"
#include <fstream>
#include <iostream>
#include <algorithm>

namespace Platformer {

RoomTemplate::RoomTemplate(std::string path) : filePath(path), role(RoomRole::UNKNOWN) {
    // Infer the RoomRole directly from the filename
    std::string lowerPath = path;
    std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(), ::tolower);
    
    if (lowerPath.find("type1") != std::string::npos) {
        role = RoomRole::TYPE_1;
    } else if (lowerPath.find("type2") != std::string::npos) {
        role = RoomRole::TYPE_2;
    } else if (lowerPath.find("type3") != std::string::npos) {
        role = RoomRole::TYPE_3;
    } else if (lowerPath.find("type0") != std::string::npos) {
        role = RoomRole::TYPE_0;
    }
}

bool RoomTemplate::load() {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open room template file: " << filePath << std::endl;
        return false;
    }

    grid.clear();
    std::string line;
    while (std::getline(file, line)) {
        // Strip out carriage returns (\r) just in case files have Windows line endings
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        if (line.empty()) continue; // Skip blank lines

        // Convert string into a vector of chars
        std::vector<char> row(line.begin(), line.end());
        grid.push_back(row);
    }

    file.close();
    return true;
}

std::vector<std::vector<char>> RoomTemplate::getGrid() const {
    return grid;
}

RoomRole RoomTemplate::getRole() const {
    return role;
}

}

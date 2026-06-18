#include "RoomTemplate.h"
#include <fstream>
#include <iostream>
#include <algorithm>

namespace Platformer {

RoomTemplate::RoomTemplate(std::string path) : filePath(path), role(RoomRole::UNKNOWN) {
    // Infer the RoomRole directly from the filename
    std::string lowerPath = path;
    std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(), ::tolower);
    
    if (lowerPath.find("path") != std::string::npos) {
        role = RoomRole::PATH;
    } else if (lowerPath.find("side") != std::string::npos) {
        role = RoomRole::SIDE;
    } else if (lowerPath.find("shop") != std::string::npos) {
        role = RoomRole::SHOP;
    } else if (lowerPath.find("treasure") != std::string::npos) {
        role = RoomRole::TREASURE;
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

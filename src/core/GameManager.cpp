#include "GameManager.h"
#include <fstream>
#include <iostream>
#include <filesystem>

namespace Platformer {

GameManager::GameManager() {
    resetRun();
}

GameManager::~GameManager() {
    // Nothing to clean up
}

GameManager* GameManager::getInstance() {
    static GameManager instance;
    return &instance;
}

int GameManager::getFloor() {
    return currentFloor;
}

int GameManager::getScore() {
    return score;
}

void GameManager::addScore(int points) {
    score += points;
}

void GameManager::nextFloor() {
    currentFloor++;
    if (currentFloor >= 7) {
        ghostTimer = 120.0f;
    } else if (currentFloor >= 4) {
        ghostTimer = 150.0f;
    } else {
        ghostTimer = 180.0f;
    }
}

void GameManager::resetRun() {
    currentFloor = 1;
    score = 0;
    playerLives = 3;
    selectedCharacter = CharacterType::EXPLORER;
    ghostTimer = 180.0f;
}

CharacterType GameManager::getSelectedCharacter() {
    return selectedCharacter;
}

void GameManager::setSelectedCharacter(CharacterType type) {
    selectedCharacter = type;
}

float GameManager::getGhostTimer() {
    return ghostTimer;
}

bool GameManager::tickGhostTimer(float dt) {
    if (ghostTimer <= 0.0f) {
        return false; // Already spawned
    }
    
    ghostTimer -= dt;
    if (ghostTimer <= 0.0f) {
        return true; // Just crossed zero
    }
    
    return false;
}

void GameManager::saveHighScore(const std::string& name) {
    std::filesystem::create_directories("saves");
    std::ofstream file("saves/highscores.sav", std::ios::app);
    if (file.is_open()) {
        file << name << "," << score << "," << currentFloor << "\n";
        file.close();
    } else {
        std::cerr << "Failed to open saves/highscores.sav for saving." << std::endl;
    }
}

}

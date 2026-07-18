#include "GameManager.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <sstream>
#include <algorithm>

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

int GameManager::getPlayerHealth() const { return playerHealth; }
int GameManager::getPlayerBombs() const { return playerBombs; }
int GameManager::getPlayerRopes() const { return playerRopes; }
int GameManager::getPlayerGold() const { return playerGold; }

void GameManager::syncPlayerStats(int hp, int b, int r, int g) {
    playerHealth = hp;
    playerBombs = b;
    playerRopes = r;
    playerGold = g;
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
    playerHealth = 4;
    playerBombs = 4;
    playerRopes = 4;
    playerGold = 0;
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

std::vector<HighScoreEntry> GameManager::loadHighScores() {
    std::vector<HighScoreEntry> scores;
    std::ifstream file("saves/highscores.sav");
    if (!file.is_open()) return scores;
    
    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string nameStr, scoreStr, floorStr;
        
        if (std::getline(ss, nameStr, ',') && 
            std::getline(ss, scoreStr, ',') && 
            std::getline(ss, floorStr)) {
            
            try {
                HighScoreEntry entry;
                entry.name = nameStr;
                entry.score = std::stoi(scoreStr);
                entry.floorsReached = std::stoi(floorStr);
                scores.push_back(entry);
            } catch (const std::exception& e) {
                // Ignore malformed lines
            }
        }
    }
    
    std::sort(scores.begin(), scores.end(), [](const HighScoreEntry& a, const HighScoreEntry& b) {
        return a.score > b.score;
    });
    
    return scores;
}

}

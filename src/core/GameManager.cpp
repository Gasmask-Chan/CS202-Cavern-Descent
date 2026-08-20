#include "GameManager.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include "../player/MovementStrategy.h"

namespace Platformer {

GameManager::GameManager() { resetRun(); }

GameManager::~GameManager() {
  // Nothing to clean up
}

GameManager *GameManager::getInstance() {
  static GameManager instance;
  return &instance;
}

int GameManager::getFloor() { return currentFloor; }

ZoneType GameManager::getZone() {
  if (currentFloor >= 3)
    return ZoneType::TEMPLE;
  if (currentFloor >= 2)
    return ZoneType::JUNGLE;
  return ZoneType::CAVE;
}

int GameManager::getScore() { return score; }

void GameManager::addScore(int points) { score += points; }

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
}

void GameManager::resetRun() {
  currentFloor = 1;
  score = 0;
  playerBombs = 4;
  playerRopes = 4;
  playerGold = 0;
  setSelectedCharacter(CharacterType::EXPLORER);
  ghostTimer = 300.0f;
  isCustomLevel = false;
}

CharacterType GameManager::getSelectedCharacter() { return selectedCharacter; }

void GameManager::setSelectedCharacter(CharacterType type) {
  selectedCharacter = type;
  
  MovementStrategy* strategy = nullptr;
  if (type == CharacterType::NINJA) strategy = new NinjaStrategy();
  else if (type == CharacterType::TANK) strategy = new TankStrategy();
  else strategy = new ExplorerStrategy();

  playerHealth = strategy->getMaxHealth();
  delete strategy;
}

float GameManager::getGhostTimer() { return ghostTimer; }
void GameManager::setGhostTimer(float timer) { ghostTimer = timer; }

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

void GameManager::saveHighScore(const std::string &name) {
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
  if (!file.is_open())
    return scores;

  std::string line;
  while (std::getline(file, line)) {
    std::stringstream ss(line);
    std::string nameStr, scoreStr, floorStr;

    if (std::getline(ss, nameStr, ',') && std::getline(ss, scoreStr, ',') &&
        std::getline(ss, floorStr)) {

      try {
        HighScoreEntry entry;
        entry.name = nameStr;
        entry.score = std::stoi(scoreStr);
        entry.floorsReached = std::stoi(floorStr);
        scores.push_back(entry);
      } catch (const std::exception &e) {
        // Ignore malformed lines
      }
    }
  }

  std::sort(scores.begin(), scores.end(),
            [](const HighScoreEntry &a, const HighScoreEntry &b) {
              return a.score > b.score;
            });

  return scores;
}

} // namespace Platformer

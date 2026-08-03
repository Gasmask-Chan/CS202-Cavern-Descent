#pragma once

#include "../Config.h"
#include <string>
#include <vector>
#include "GameState.h"

namespace Platformer {

class GameManager {
private:
    int currentFloor;
    int score;
    int playerLives;
    int playerHealth;
    int playerBombs;
    int playerRopes;
    int playerGold;
    CharacterType selectedCharacter;
    float ghostTimer;
    bool isCustomLevel = false;
    bool loadIntoEditor = false;
    std::string customLevelPath = "levels/custom_level.lvl";
    // FloorModifier currentModifier;
    
    GameManager();
    ~GameManager();
public:
    static GameManager* getInstance();

    GameManager(const GameManager &) = delete;
    GameManager& operator=(const GameManager &) = delete;

    bool getIsCustomLevel() const { return isCustomLevel; }
    void setIsCustomLevel(bool b) { isCustomLevel = b; }

    bool getLoadIntoEditor() const { return loadIntoEditor; }
    void setLoadIntoEditor(bool b) { loadIntoEditor = b; }

    std::string getCustomLevelPath() const { return customLevelPath; }
    void setCustomLevelPath(const std::string& p) { customLevelPath = p; }

    int getFloor();
    ZoneType getZone();

    int getScore();
    void addScore(int points);
    
    int getPlayerHealth() const;
    int getPlayerBombs() const;
    int getPlayerRopes() const;
    int getPlayerGold() const;
    void syncPlayerStats(int hp, int b, int r, int g);

    /**
     * @brief Increments `currentFloor` by 1. Determines `ZoneType` from floor number (1–3=Cave, 4–6=Jungle, 7–9=Temple). Resets `ghostTimer` to the new floor's timer value from `DifficultyConfig`.
     * 
     */
    void nextFloor();

    /**
     * @brief Resets all run state to defaults: `currentFloor=1`, `score=0`, `playerLives=3`, `ghostTimer=180`. Called on permadeath before starting a new run.
     * 
     */
    void resetRun();

    CharacterType getSelectedCharacter();

    void setSelectedCharacter(CharacterType type);

    float getGhostTimer();

    /**
     * @brief Decrements `ghostTimer` by `dt`. Returns `true` when timer reaches zero (signals ghost spawn). Timer stops decrementing once ghost is active.
     * 
     * @param dt 
     * @return true, false
     */
    bool tickGhostTimer(float dt);

    // FloorModifier getFloorModifier();

    void saveHighScore(const std::string &name);

    std::vector<HighScoreEntry> loadHighScores();
};

}
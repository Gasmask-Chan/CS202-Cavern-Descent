#pragma once

#include "../Config.h"
#include "GameState.h"
#include "string"

namespace Platformer {

class GameManager {
private:
    static GameManager* instance;
    int currentFloor;
    int score;
    int playerLives;
    CharacterType selectedCharacter;
    float ghostTimer;
    // FloorModifier currentModifier;
    
    GameManager();
    ~GameManager();
public:
    static GameManager* getInstance();

    GameManager(const GameManager &) = delete;
    GameManager& operator=(const GameManager &) = delete;

    int getFloor();

    int getScore();

    /**
     * @brief Adds `points` to the running `score` total. Called when enemies are killed or treasure is collected.
     * 
     * @param points 
     * @return int 
     */
    void addScore(int points);

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

    // std::vector<HighScoreEntry> loadHighScores();
};

}
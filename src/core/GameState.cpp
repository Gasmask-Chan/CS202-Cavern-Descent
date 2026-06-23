#include "GameState.h"
#include "Game.h"

namespace Platformer {

void GameState::setGame(Game* game) {
    this->game = game;
}

/*
=======================================================
=========================MENU==========================
=======================================================
*/

void MenuState::enter() {

}

void MenuState::exit() {

}

void MenuState::handleInput() {

}

void MenuState::update(float dt) {

}

void MenuState::render() {
    ClearBackground(GREEN);
}

/*
=======================================================
=========================PLAY==========================
=======================================================
*/

void PlayState::enter() {

}

void PlayState::exit() {

}

void PlayState::handleInput() {

}

void PlayState::update(float dt) {

}

void PlayState::render() {
    ClearBackground(GREEN);
}

/*
=======================================================
=========================PAUSE=========================
=======================================================
*/

void PauseState::enter() {

}

void PauseState::exit() {

}

void PauseState::handleInput() {

}

void PauseState::update(float dt) {

}

void PauseState::render() {
    ClearBackground(GREEN);
}

/*
=======================================================
=========================GAMEOVER======================
=======================================================
*/

void GameOverState::enter() {

}

void GameOverState::exit() {

}

void GameOverState::handleInput() {

}

void GameOverState::update(float dt) {

}

void GameOverState::render() {
    ClearBackground(GREEN);
}

/*
=======================================================
=========================CHARSELECT====================
=======================================================
*/

void CharSelectState::enter() {
    selectedIndex = 0;
}

void CharSelectState::exit() {

}

void CharSelectState::handleInput() {
    if (IsKeyPressed(KEY_LEFT)) {
        selectedIndex = (selectedIndex + 2) % 3;
    }
    if (IsKeyPressed(KEY_RIGHT)) {
        selectedIndex = (selectedIndex + 1) % 3;
    }
    if (IsKeyPressed(KEY_ENTER)) {
        // TODO: Pass characters[selectedIndex] to GameManager so the correct Player strategy is spawned
        game->changeState(GameStateType::PLAY);
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        game->changeState(GameStateType::MENU);
    }
}

void CharSelectState::update(float dt) {

}

void CharSelectState::render() {
    ClearBackground(BLACK);
    
    DrawText("CHARACTER SELECT", 450, 200, 40, RAYWHITE);
    
    Color expColor = (selectedIndex == 0) ? YELLOW : DARKGRAY;
    Color ninColor = (selectedIndex == 1) ? BLUE : DARKGRAY;
    Color tnkColor = (selectedIndex == 2) ? RED : DARKGRAY;
    
    DrawText("EXPLORER", 300, 400, 30, expColor);
    DrawText("NINJA", 600, 400, 30, ninColor);
    DrawText("TANK", 900, 400, 30, tnkColor);
    
    DrawText("Press ENTER to start, ESC to return", 420, 600, 20, GRAY);
}

/*
=======================================================
=========================EDITOR====================
=======================================================
*/

void EditorState::enter() {

}

void EditorState::exit() {

}

void EditorState::handleInput() {

}

void EditorState::update(float dt) {

}

void EditorState::render() {
    ClearBackground(GREEN);
}

}
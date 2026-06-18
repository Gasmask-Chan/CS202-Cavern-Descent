#include "GameState.h"

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

}

void CharSelectState::exit() {

}

void CharSelectState::handleInput() {

}

void CharSelectState::update(float dt) {

}

void CharSelectState::render() {
    ClearBackground(GREEN);
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
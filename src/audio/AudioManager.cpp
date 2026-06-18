#include "AudioManager.h"
#include <iostream>
#include <algorithm>

namespace Platformer {

AudioManager* AudioManager::instance = nullptr;

AudioManager::AudioManager() : isBgmPlaying(false), sfxVolume(1.0f), bgmVolume(1.0f) {
    InitAudioDevice(); // Required by Raylib before any audio loading/playing
}

AudioManager::~AudioManager() {
    // Free all loaded SFX
    for (auto& pair : sfxCache) {
        UnloadSound(pair.second);
    }
    sfxCache.clear();

    // Free loaded BGM
    if (isBgmPlaying) {
        StopMusicStream(currentBGM);
        UnloadMusicStream(currentBGM);
    }
    
    CloseAudioDevice();
}

AudioManager* AudioManager::getInstance() {
    if (instance == nullptr) {
        instance = new AudioManager();
    }
    return instance;
}

void AudioManager::loadSFX(const std::string& name, const std::string& filePath) {
    Sound sound = LoadSound(filePath.c_str());
    // In raylib, a successful load has a non-null buffer
    if (sound.stream.buffer != nullptr) {
        SetSoundVolume(sound, sfxVolume);
        sfxCache[name] = sound;
    } else {
        std::cerr << "AudioManager: Failed to load SFX: " << filePath << std::endl;
    }
}

void AudioManager::playSFX(const std::string& name) {
    auto it = sfxCache.find(name);
    if (it != sfxCache.end()) {
        SetSoundVolume(it->second, sfxVolume); // Ensure current volume is applied
        PlaySound(it->second);
    } else {
        std::cerr << "AudioManager Warning: SFX not found in cache: " << name << std::endl;
    }
}

void AudioManager::playBGM(const std::string& filePath) {
    if (isBgmPlaying) {
        StopMusicStream(currentBGM);
        UnloadMusicStream(currentBGM);
    }

    currentBGM = LoadMusicStream(filePath.c_str());
    if (currentBGM.stream.buffer != nullptr) {
        SetMusicVolume(currentBGM, bgmVolume);
        PlayMusicStream(currentBGM);
        isBgmPlaying = true;
    } else {
        std::cerr << "AudioManager Warning: Failed to load BGM: " << filePath << std::endl;
        isBgmPlaying = false;
    }
}

void AudioManager::stopBGM() {
    if (isBgmPlaying) {
        StopMusicStream(currentBGM);
    }
}

void AudioManager::updateBGM() {
    // Raylib requires this to be called every frame to stream the music
    if (isBgmPlaying) {
        UpdateMusicStream(currentBGM);
    }
}

void AudioManager::setVolume(float sfx, float bgm) {
    // Clamp volumes between 0.0 and 1.0 (0% to 100%)
    sfxVolume = std::clamp(sfx, 0.0f, 1.0f);
    bgmVolume = std::clamp(bgm, 0.0f, 1.0f);

    // Immediately apply to currently playing BGM
    if (isBgmPlaying) {
        SetMusicVolume(currentBGM, bgmVolume);
    }
}

}

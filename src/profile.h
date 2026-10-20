#pragma once

#include <string>
#include <vector>
#include "constants.h"

inline constexpr int MAX_PROFILES = 5; // Cap at 5 profiles so they can fit on the screen (scroll is beyond scope)

// Struct for holding player data
struct PlayerProfile {
    std::string initials; // Player initials
    int highScore = 0; // Player's high score
    int gamesPlayed = 0; // How many games the player has played
    bool soundOn = true; // The player's saved sound setting
    int speedSetting = SPEED_NORMAL; // The player's saved speed setting
};

// Struct used to save profiles locally
struct ProfileStore {
    std::vector<PlayerProfile> profiles;
    int current = -1; // Index of currently active profile, or -1 if there isn't one

    static ProfileStore Load(const std::string &filepath);
    void Save(const std::string &filepath);
    void SelectOrCreate(const std::string &initials);
    bool CanAddPlayer() const;

    PlayerProfile &Current(); // Read/write access
    const PlayerProfile &Current() const; // Read-only access variation
};
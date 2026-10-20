#pragma once

#include <string>
#include <vector>

#include "constants.h"

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
};

ProfileStore LoadProfiles(const std::string &filepath);
void SaveProfiles(const ProfileStore &store, const std::string &filepath);
void SelectOrCreateProfile(ProfileStore &store, const std::string &initials);

PlayerProfile &CurrentProfile(ProfileStore &store); // Read/write access
const PlayerProfile &CurrentProfile(const ProfileStore &store); // Read-only access variation
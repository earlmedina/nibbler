#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "raylib.h"
#include "profile.h"
#include "ui.h"

constexpr int MIN_INITIALS_LENGTH = 2; // Require at least 2 initials
constexpr int MAX_INITIALS_LENGTH = 3; // Don't allow more than 3 initials
constexpr int MAX_PROFILES = 5; // Cap at 5 profiles so they can fit on the screen (scroll is beyond scope)

// Struct for initials input.
struct InitialsScreen {
    std::string text; // Initials value
    bool canCancel = false; // Default to false to force initials input on first run
};

enum class InitialsResult { None, Submitted, Canceled }; // Enum denoting initials entry result status

InitialsResult UpdateInitialsScreen(InitialsScreen &screen, std::unordered_map<std::string, Sound> &sounds);
void DrawInitialsScreen(const InitialsScreen &screen, std::unordered_map<std::string, Texture2D> &keyTextures, Font &font);

std::vector<MenuItem> LayoutPlayerMenu(const ProfileStore &store);
bool CanAddPlayer(const ProfileStore &store);


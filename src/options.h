#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include "raylib.h"
#include "toggle.h"
#include "ui.h"

// The Options screen is more complicated than other screens, so it makes more sense to model it.
struct OptionsScreen {
    // In order for keyboard navigation to work on Options screen, need to know which row is active
    // Can add a count value at the end as a simple way to get number of options:
    // https://stackoverflow.com/questions/2181933/is-there-a-way-to-find-the-cardinality-size-of-an-enum-in-c
    enum OptionsRow { SOUND, SPEED, BACK, COUNT };
    int focusedRow = SOUND; // Initially, the first row is focused
    ToggleButton soundToggle;
    ToggleGroup speedGroup;
    std::vector<MenuItem> optionsScreenMenuItems;
};

OptionsScreen CreateOptionsScreen(bool soundOn, int selectedSpeedSetting);
bool UpdateOptionsScreen(OptionsScreen &screen, const std::unordered_map<std::string, Sound> &sounds);
void DrawOptionsScreen(const OptionsScreen &screen, const Font &font);
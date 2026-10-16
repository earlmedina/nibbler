#pragma once
#include <string>
#include <vector>
#include "raylib.h"

/**
 * Struct representing toggle button used in Options screen
 */
struct ToggleButton {
    std::string label; // button label
    Rectangle rect; // Rectangle representing where the button should be drawn
    bool isToggled = false; // Bool value indicating whether the button is toggled
};

/**
 * Struct representing a group of toggle buttons (acts like a radio button)
 */
struct ToggleGroup {
    std::vector<ToggleButton> buttons; // toggle buttons that make up group
    int selected = 0; // The index corresponding to the selected toggle button in the group
};

ToggleGroup CreateToggleGroup(const std::vector<std::string> &buttonLabels, Rectangle bounds, int selected = 0);

bool UpdateToggleButton(ToggleButton &toggleButton);
void DrawToggleButton(const ToggleButton &toggleButton);

bool UpdateTogglGroup(ToggleGroup &togglGroup);
void DrawToggleGroup(const ToggleGroup &toggleGroup);
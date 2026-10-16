#include "toggle.h"
#include <algorithm>

// Unnamed namespace used to keep the contained private
// https://medium.com/@mane.tako/understanding-unnamed-namespaces-in-c-7d41d367fd47
namespace {
    constexpr Color TOGGLE_OFF_COLOR = { 255, 255, 255, 15 };
    constexpr Color TOGGLE_ON_COLOR = { 255, 200, 0, 60 };
    constexpr Color TOGGLE_HOVER_COLOR = { 255, 255, 255, 35 };
    constexpr float TOGGLE_ROUNDNESS = 0.3f;
    constexpr float TOGGLE_GROUP_BUTTON_GAP = 8.0f;
    constexpr int TOGGLE_FONT_SIZE = 24;

    // Keyboard constants
    constexpr Color TOGGLE_FOCUS_COLOR = {102, 191, 255, 100 };
    constexpr float FOCUS_GAP = 6.0f;

    /**
     * Draws outline around toggle button. Used for keyboard focus.
     * @param rect - the Rectangle to draw an outline around
     */
    void DrawFocusOutline(const Rectangle rect) {
        const Rectangle outline = { rect.x - FOCUS_GAP, rect.y - FOCUS_GAP,
            rect.width + 2* FOCUS_GAP, rect.height + 2 * FOCUS_GAP };
        DrawRectangleRoundedLines(outline, TOGGLE_ROUNDNESS, 8, TOGGLE_FOCUS_COLOR);
    }
}

/**
 * Updates the toggle status if clicked/keyed.
 * @param toggleButton - reference to Toggle Button
 * @param hasKeyboardSelect - bool value indicating if a selection key was pressed (used for keyboard toggle)
 * @return - true if the Toggle Button changed (used for playing sounds)
 */
bool UpdateToggleButton(ToggleButton &toggleButton, bool hasKeyboardSelect) {
    // If the Toggle Button is clicked, update isToggled.
    const bool clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), toggleButton.rect);
    if (clicked || hasKeyboardSelect) {
        toggleButton.isToggled = !toggleButton.isToggled;
        return true;
    }
    return false;
}

/**
 * Draws the specified Toggle Button
 * @param toggleButton - reference to Toggle Button struct
 * @param hasFocus - indicates whether the button has focus from keyboard and should have focus outline drawn
 */
void DrawToggleButton(const ToggleButton &toggleButton, bool hasFocus) {
    // Check if button has hover
    const bool hasHover = CheckCollisionPointRec(GetMousePosition(), toggleButton.rect);

    // Update button color based on toggle status and hover
    const Color btnFill = toggleButton.isToggled ? TOGGLE_ON_COLOR : hasHover ? TOGGLE_HOVER_COLOR : TOGGLE_OFF_COLOR;
    const Color lineFill = toggleButton.isToggled ? GOLD : Fade(RAYWHITE, 0.4f);
    const Color labelColor = toggleButton.isToggled ? GOLD : RAYWHITE;

    // Draw toggle button
    DrawRectangleRounded(toggleButton.rect, TOGGLE_ROUNDNESS, 8, btnFill);
    DrawRectangleRoundedLines(toggleButton.rect, TOGGLE_ROUNDNESS, 8, lineFill);

    // Draw centered label
    const int textWidth = MeasureText(toggleButton.label.c_str(), TOGGLE_FONT_SIZE);
    const int startX = static_cast<int>(toggleButton.rect.x + (toggleButton.rect.width - textWidth) / 2);
    const int startY = static_cast<int>(toggleButton.rect.y + (toggleButton.rect.height - TOGGLE_FONT_SIZE) / 2);
    DrawText(toggleButton.label.c_str(), startX, startY, TOGGLE_FONT_SIZE, labelColor);

    if (hasFocus)
        DrawFocusOutline(toggleButton.rect);
}

/**
 * Creates a toggle group with the specified bounds, and creates toggle buttons using provided button labels.
 * @param buttonLabels - label text for each button
 * @param bounds - Rectangle representing where the group should be drawn
 * @param selected - int value representing the index of the selected toggle button
 * @return ToggleGroup struct
 */
ToggleGroup CreateToggleGroup(const std::vector<std::string> &buttonLabels, Rectangle bounds, int selected) {
    // Create ToggleGroup struct to return
    ToggleGroup group;
    group.selected = selected;
    group.rect = bounds;

    //** Set buttons **
    // Determine button width
    const float buttonCount = static_cast<float>(buttonLabels.size());
    const float btnWidth = (bounds.width - TOGGLE_GROUP_BUTTON_GAP * (buttonCount - 1)) / buttonCount;

    // Dynamically set button rects
    for (size_t i = 0; i < buttonLabels.size(); i++) {
        const Rectangle rect = { bounds.x + static_cast<float>(i) * (btnWidth + TOGGLE_GROUP_BUTTON_GAP),
            bounds.y, btnWidth, bounds.height };
        group.buttons.push_back({ buttonLabels[i], rect, static_cast<int>(i) == selected});
    }

    return group;
}

/**
 * Handles toggle status for Toggle Buttons in Toggle Group
 * @param toggleGroup - reference to Toggle Group
 * @param keyShift - Values are -1, 0, or 1 and represent how much the selected index
 * should be shifted due in response to keyboard navigation
 * @return true if the Toggle Group selection changed
 */
bool UpdateToggleGroup(ToggleGroup &toggleGroup, int keyShift) {
    // Get the last index in the buttons array for upper limit
    const int lastIndex = static_cast<int>(toggleGroup.buttons.size()) - 1;

    // Get the new selected index after the shift. Here clamp is used to make sure we dont
    // go out of the array bounds. If lower, than 0 we stay at 0, if higher than the last index
    // we stay at the last index: https://en.cppreference.com/cpp/algorithm/clamp
    int newSelected = std::clamp(toggleGroup.selected + keyShift, 0, lastIndex);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        const Vector2 mPos = GetMousePosition();
        for (int i = 0; i <= lastIndex; i++) {
            // If there was a mouse selection, update newSelected
            if (CheckCollisionPointRec(mPos, toggleGroup.buttons[i].rect))
                newSelected = i;
        }
    }

    // If the selection didn't actually change, return false
    if (newSelected == toggleGroup.selected)
        return false;

    // Otherwise, update selection
    toggleGroup.buttons[toggleGroup.selected].isToggled = false;
    toggleGroup.buttons[newSelected].isToggled = true;
    toggleGroup.selected = newSelected;

    return true;
}

/**
 * Draws the specified Toggle Group
 * @param toggleGroup - reference to toggle group
 * @param hasFocus - indicates whether the group has keyboard focus (if true draw focus outline)
 */
void DrawToggleGroup(const ToggleGroup &toggleGroup, bool hasFocus) {
    for (const ToggleButton &button : toggleGroup.buttons)
        DrawToggleButton(button);

    // If the group has focus, draw outline (guard against empty buttons array)
    if (hasFocus && !toggleGroup.buttons.empty())
        DrawFocusOutline(toggleGroup.rect);
}
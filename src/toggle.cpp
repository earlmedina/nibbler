#include "toggle.h"

namespace {
    constexpr Color TOGGLE_OFF_COLOR = { 255, 255, 255, 15 };
    constexpr Color TOGGLE_ON_COLOR = { 255, 200, 0, 60 };
    constexpr Color TOGGLE_HOVER_COLOR = { 255, 255, 255, 35 };
    constexpr int TOGGLE_FONT_SIZE = 24;
    constexpr float TOGGLE_ROUNDNESS = 0.5f;
}

/**
 * Updates the toggle status if clicked/keyed.
 * @param toggleButton - reference to Toggle Button
 * @return - true if the Toggle Button changed (used for playing sounds)
 */
bool UpdateToggleButton(ToggleButton &toggleButton) {
    // If the Toggle Button is clicked, update isToggled.
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), toggleButton.rect)) {
        toggleButton.isToggled = !toggleButton.isToggled;
        return true;
    }
    return false;
}

/**
 * Draws the specified Toggle Button
 * @param toggleButton - reference to Toggle Button struct
 */
void DrawToggleButton(const ToggleButton &toggleButton) {
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

    //** Set buttons **
    // Determine button width
    const float btnWidth = bounds.width / static_cast<float>(buttonLabels.size());

    // Dynamically set button rects
    for (size_t i = 0; i < buttonLabels.size(); i++) {
        const Rectangle rect = { bounds.x + static_cast<float>(i) * btnWidth,
            bounds.y, btnWidth, bounds.height };
        group.buttons.push_back({ buttonLabels[i], rect, static_cast<int>(i) == selected});
    }
    return group;
}

/**
 * Handles toggle status for Toggle Buttons in Toggle Group
 * @param toggleGroup
 * @return true if the Toggle Group selection changed
 */
bool UpdateToggleGroup(ToggleGroup &toggleGroup) {
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        return false; // early return if the mouse hasn't been clicked
    }

    const Vector2 mPos = GetMousePosition();
    for (int i = 0; i < static_cast<int>(toggleGroup.buttons.size()); i++) {
        if (CheckCollisionPointRec(mPos, toggleGroup.buttons[i].rect)) {
            if (toggleGroup.selected == i)
                return false; // User selected the same item, so it's not a true update

            // Otherwise, TOGGLE OFF the button that was previously selected
            toggleGroup.buttons[toggleGroup.selected].isToggled = false;

            // TOGGLE ON the button that was just selected
            toggleGroup.buttons[i].isToggled = true;

            // Update selected value with new index
            toggleGroup.selected = i;
            return true;
        }
    }
    return false;
}

/**
 * Draws the specified Toggle Group
 * @param toggleGroup - reference to toggle group
 */
void DrawToggleGroup(const ToggleGroup &toggleGroup) {
    for (const ToggleButton &button : toggleGroup.buttons)
        DrawToggleButton(button);
}
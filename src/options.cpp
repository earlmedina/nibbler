#include "options.h"
#include "constants.h"
#include "resources.h"

// Private
namespace {
    constexpr int OPTION_LABEL_X = 80; // Start x for option labels

    /**
     *  Used to get row rects from Struct. Not great design, but it should work fine assuming
     *  more settings aren't added.
     * @param screen - reference to OptionsScreen struct
     * @return array of Rectangles corresponding to rows
     */
    std::vector<Rectangle> GetRowRects(const OptionsScreen &screen) {
        return { screen.soundToggle.rect, screen.speedGroup.rect, screen.optionsScreenMenuItems[0].rect };
    }

    /**
     * Used to draw settings in a consistent manner (heading with description underneath, toggle)
     * @param heading - The setting heading
     * @param description - Brief description of what the settings does/is for
     * @param controlRect - The Rectangle corresponding to the toggle or toggle group
     * @param font - the description font (for better readability at smaller font sizes)
     */
    void DrawOptionLabel(const char *heading, const std::initializer_list<const char *> description,
                         const Rectangle controlRect, const Font &font) {
        DrawText(heading, OPTION_LABEL_X, static_cast<int>(controlRect.y) - 20, OPTIONS_HEADING_FONT_SIZE, GOLD);
        DrawTextLinesEx(OPTION_LABEL_X, controlRect.y + 14.0f, description, font); // Can recycle this function used for hints
    }
}

/**
 * Creates OptionsScreen struct to use for drawing
 * @param soundOn - true if sound should be played
 * @param selectedSpeedSetting - corresponds to selected speed setting (0-Slow, 1-Normal, 2-Fast)
 * @return OptionsScreen struct
 */
OptionsScreen CreateOptionsScreen(bool soundOn, int selectedSpeedSetting) {
    OptionsScreen screen;
    screen.soundToggle = { soundOn ? "ON" : "OFF",
{ 320.0f, 220.0f, 400.0f, 40.0f }, soundOn };
    screen.speedGroup = CreateToggleGroup({ "Slow", "Normal", "Fast" },
{ 320.0f, 340.0f, 400.0f, 40.0f }, selectedSpeedSetting);

    // Back menu item
    screen.optionsScreenMenuItems = { { "Back", {}, true } };
    LayoutMenu(screen.optionsScreenMenuItems, 470.0f, 200.0f, 52.0f, 0.0f);
    return screen;
}

/**
 * Handle Update operations for OptionsScreen
 * @param screen - reference to OptionsScreen
 * @param sounds - reference to Sounds map (to play sounds)
 * @return True if back was selected, false otherwise (used to know whether to change currentScreen)
 */
bool UpdateOptionsScreen(OptionsScreen &screen, const std::unordered_map<std::string, Sound> &sounds) {
    // If the row focus changed, play move sound
    if (UpdateRowFocus(screen.focusedRow, GetRowRects(screen)))
        PlayMapSound(sounds, "move");

    // Check if "confirm" key was pressed
    const bool hasConfirm = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) || IsKeyPressed(KEY_SPACE);

    // Handle Sound Toggle
    if (UpdateToggleButton(screen.soundToggle, screen.focusedRow == OptionsScreen::SOUND && hasConfirm)) {
        screen.soundToggle.label = screen.soundToggle.isToggled ? "ON" : "OFF"; // Change sound toggle label according to isToggled
        SetMasterVolume(screen.soundToggle.isToggled ? 1.0f : 0.0f);
        PlayMapSound(sounds, "select");
    }

    // Handle Speed Toggle Group (left/right to navigate group buttons)
    int shift = 0;
    if (screen.focusedRow == OptionsScreen::SPEED) {
        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))
            shift = -1;
        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
            shift = 1;
    }

    // Handle Speed Toggle Buton updates
    if (UpdateToggleGroup(screen.speedGroup, shift))
        PlayMapSound(sounds, "select");

    // Handle Back menu item select
    const bool backClicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(GetMousePosition(), screen.optionsScreenMenuItems[0].rect);

    // If Back menu item was clicked or selected by keyboard or ESC/BackSpace was keyed, go back to start menu.
    if ((screen.focusedRow == OptionsScreen::BACK && hasConfirm) || backClicked ||
        IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE)) {
        PlayMapSound(sounds, "back");
        screen.focusedRow = OptionsScreen::SOUND; // Reset focus to first item
        return true;
        }
    return false;
}

void DrawOptionsScreen(const OptionsScreen &screen, const Font &font) {
    DrawCenteredText("Options", 60, 48, SCREEN_TITLE_COLOR);
    DrawCenteredText("Customize your gaming experience.", 130, 20, LIGHTGRAY);
    //DrawCenteredText("Settings are saved to your player profile.", 150, 20, LIGHTGRAY);

    DrawOptionLabel("Sound Effects", { "Toggle navigation and","gameplay sound effects." },
        screen.soundToggle.rect, font);

    DrawToggleButton(screen.soundToggle, screen.focusedRow == OptionsScreen::SOUND);

    DrawOptionLabel("Snake Speed", { "Set the starting snake", "speed." },
        screen.speedGroup.rect, font);
    DrawToggleGroup(screen.speedGroup, screen.focusedRow == OptionsScreen::SPEED);
    DrawMenuItems(screen.optionsScreenMenuItems, screen.focusedRow == OptionsScreen::BACK ? 0 : -1);
}

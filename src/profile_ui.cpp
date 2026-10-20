#include "profile_ui.h"
#include "constants.h"
#include "resources.h"

// Private data
namespace {
    // Constants for character box
    constexpr float CHAR_BOX_WIDTH = 64.0f;
    constexpr float CHAR_BOX_HEIGHT = 76.0f;
    constexpr float CHAR_BOX_GAP = 16.0f;
    constexpr float CHAR_BOX_Y = 240.0f;
    constexpr int CHAR_FONT_SIZE = 48;

    /**
     * Checks if input is a valid a-z, A-Z character
     * @param key - Key as character (interpreted as int)
     * @return true if alpha value
     */
    bool IsLetter(int key) {
        return (key >= 'a' && key <= 'z') || (key >= 'A' && key <= 'Z');
    }

    /**
     * Converts GetCharPressed() value to Uppercase char
     * -Note: assume input has been validated
     * @param key - Value return by GetCharPressed()
     * @return - key value as upper case char
     */
    char ToUpperChar(int key) {
        return static_cast<char>(key >= 'a' ? key - 'a' + 'A' : key);
    }
}

/**
 * Updates the initials screen input
 * @param screen - reference to InputScreen
 * @param sounds - reference to sounds map
 * @return InitialsResult
 */
InitialsResult UpdateInitialsScreen(InitialsScreen &screen, std::unordered_map<std::string, Sound> &sounds) {
    int key = GetCharPressed(); // Get char pressed
    while (key > 0) {
        // Check if letter key and max 3 chars in length
        if (IsLetter(key) && static_cast<int>(screen.text.size()) < MAX_INITIALS_LENGTH) {
            screen.text.push_back(ToUpperChar(key)); // Push uppercase valid value to text member
            PlayMapSound(sounds, "move"); // Play sound to provide user with feedback
        }
        key = GetCharPressed(); // Get next character
    }
    // Handle erase via backspace/delete
    if ((IsKeyPressed(KEY_BACKSPACE) || IsKeyPressed(KEY_DELETE)) && !screen.text.empty()) {
        screen.text.pop_back(); // Pop last character to erase
        PlayMapSound(sounds, "back"); // Play sound to provide user with feedback
    }

    // Handle submit - initials should be 2-3 characters
    const bool hasSubmit = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER);

    const int textLength = static_cast<int>(screen.text.size());
    // If Enter has been pressed and input is <= max length, return Submitted result
    if (hasSubmit && textLength >= MIN_INITIALS_LENGTH && textLength <= MAX_INITIALS_LENGTH) {
        PlayMapSound(sounds, "select");
        return InitialsResult::Submitted;
    }

    // Handle cancel
    if (screen.canCancel && IsKeyPressed(KEY_ESCAPE)) {
        PlayMapSound(sounds, "back");
        return InitialsResult::Canceled;
    }

    // Otherwise, return None (something unexpected happened)
    return InitialsResult::None;
}

/**
 * Draws Initials Screen
 * @param screen - reference to InitialsScreen
 */
void DrawInitialsScreen(const InitialsScreen &screen, std::unordered_map<std::string, Texture2D> &keyTextures, Font &font) {
    DrawCenteredText("New Player", 60, 48, SCREEN_TITLE_COLOR); // Draw title
    DrawCenteredText("Enter your initials to personalize your experience.", 140, 24, LIGHTGRAY); // Draw subtitle

    // Calculate width of text input box (including gaps between chars)
    const float textInputWidth = MAX_INITIALS_LENGTH * CHAR_BOX_WIDTH + (MAX_INITIALS_LENGTH - 1) * CHAR_BOX_GAP;
    const float startX = (SCREEN_WIDTH - textInputWidth) / 2.0f; // Center X for text input box
    const int currCursorIdx = static_cast<int>(screen.text.size()); // The current position where the cursor should be drawn

    for (int i = 0; i < MAX_INITIALS_LENGTH; i++) {
        // Get current character box Rectangle
        const Rectangle charBox = { startX + static_cast<float>(i) * (CHAR_BOX_WIDTH + CHAR_BOX_GAP),
            CHAR_BOX_Y, CHAR_BOX_WIDTH, CHAR_BOX_HEIGHT };
        const bool isCursor = (i == currCursorIdx); // Use to draw cursor at i

        // Draw grayish background for char, green for cursor
        const auto t = static_cast<float>(GetTime()); // Needs to be cast to float for sinf
        float alpha = (cosf(t * 6.0f) * 0.5f) + 0.5f;
        DrawRectangleRounded(charBox, 0.2f, 8, isCursor ? Fade(GREEN, alpha) : Fade(RAYWHITE, 0.4f));

        // Draw letter
        if (i < currCursorIdx) {
            // Get current char
            // Note: screen.text[i] return char, so we need to null terminate it
            const char letter[2] = { screen.text[i], '\0'}; // Init size 2 char array for letter and null term.
            const int letterWidth = MeasureText(letter, CHAR_FONT_SIZE); // Get letter width
            DrawText(letter, static_cast<int>(charBox.x + (charBox.width - static_cast<float>(letterWidth)) / 2),
                static_cast<int>(charBox.y + (charBox.height - CHAR_FONT_SIZE) / 2), CHAR_FONT_SIZE, RAYWHITE);
        }
    }

    // Draw basic hint text
    DrawCenteredHintLine(INITIALS_TYPE, 380, keyTextures, font);
}

std::vector<MenuItem> LayoutPlayerMenu(const ProfileStore &store) {
    std::vector<MenuItem> menuItems; // Declare array to return

    // Create menu items from initials
    for (const PlayerProfile &p : store.profiles)
        menuItems.push_back({ p.initials, {} });

    // Add "Add New Player" if we're under the max
    if (store.CanAddPlayer())
        menuItems.push_back({ "(+) New Player", {}, MenuItemType::Special });

    // Add "Back" button
    menuItems.push_back({ "Back", {}, MenuItemType::Back });
    LayoutMenu(menuItems, 200.0f, 320.0f, 44.0f, 54.0f);
    return menuItems;
}
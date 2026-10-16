#include <deque>
#include <raylib.h>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

#include "cell.h"
#include "constants.h"
#include "food.h"
#include "game.h"
#include "resources.h"
#include "snake.h"
#include "toggle.h"
#include "ui.h"


int main() {
    const char *title = "Nibbler";
    const char *subTitle = "A quick, fun classic arcade game";
    SetConfigFlags(FLAG_WINDOW_HIGHDPI); // Looks blurry on dev machine otherwise
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, title);
    InitAudioDevice();
    ChangeDirectory(GetApplicationDirectory()); // For proper font loading
    SetTargetFPS(TARGET_FPS);
    SetExitKey(KEY_NULL); // Override default

    // Load how to font
    int howToFontSize = static_cast<int>(roundf(32 * GetWindowScaleDPI().x)); // For better scaling
    Font howToFont = LoadFontEx("resources/Menlo-Regular.ttf", howToFontSize, nullptr, 0);
    GenTextureMipmaps(&howToFont.texture);
    SetTextureFilter(howToFont.texture, TEXTURE_FILTER_POINT);

    // Load key textures
    std::unordered_map<std::string, Texture2D> keyTextures = LoadKeyTextures();

    // Load sounds
    std::unordered_map<std::string, Sound> sounds = LoadSounds();

    Screen currentScreen = Screen::StartMenu; // Set start screen to Start Menu

    /************ MENU SETUP ************/
    // Create array to store menu options and then set placement values

    /*** Start Menu ***/
    std::vector<MenuItem> startMenuItems = {
    { "Play", {} },
    { "How to Play", {} },
    { "Options", {} },
    { "Quit", {} }
    };

    // Set placement
    LayoutMenu(startMenuItems, 220.0f, 320.0f, 56.0f, 72.0f);
    int selectedStartMenuItem = 0; // By default, first item (Play) is selected.

    /*** How to Play ***/
    std::vector<MenuItem> howToPlayItems = {
    {"Back", {}, true },
    };

    LayoutMenu(howToPlayItems, 470.0f, 200.0f, 52.0f, 0.0f);
    int selectedHowToPlayItem = 0;

    /*** Game Over ***/
    std::vector<MenuItem> gameOverItems = {
        {"Play Again", {} },
        {"Start Menu", {}, true },
    };

    LayoutMenu(gameOverItems, 350.0f, 320.0f, 56.0f, 72.0f);
    int selectedGameOverItem = 0;

    /*** Quit Prompt ***/
    std::vector<MenuItem> quitPromptItems = {
        {"Yes", {} },
        {"No", {}, true }
    };
    LayoutMenu(quitPromptItems, 270.0f, 240.0f, 52.0f, 68.0f);
    int selectedQuitPromptItem = 1;

    /*** Options ***/
    // In order for keyboard navigation to work on Options screen, need to know which row is active
    // Can add a count value at the end as a simple way to get number of options:
    // https://stackoverflow.com/questions/2181933/is-there-a-way-to-find-the-cardinality-size-of-an-enum-in-c
    enum OptionsRow { OPTIONS_SOUND, OPTIONS_SPEED, OPTIONS_BACK, OPTIONS_COUNT };
    int focusedOptionRow = OPTIONS_SOUND; // First row gets focus

    // Init settings toggles
    bool soundOn = true; // UPDATE WITH MICROSERVICE LATER
    ToggleButton soundToggle = { soundOn ? "ON" : "OFF",
{ 320.0f, 220.0f, 400.0f, 40.0f }, soundOn };

    enum SpeedSetting { SLOW, NORMAL, FAST };
    int selectedSpeedSetting = NORMAL;  // UPDATE WITH MICROSERVICE LATER
    ToggleGroup speedToggleGroup = CreateToggleGroup({ "Slow", "Normal", "Fast" },
{ 320.0f, 340.0f, 400.0f, 40.0f }, selectedSpeedSetting);

    // Back menu item
    std::vector<MenuItem> optionsMenuItems = { { "Back", {}, true } };
    LayoutMenu(optionsMenuItems, 470.0f, 200.0f, 52.0f, 0.0f);

    /*** Game Vars ***/
    int highScore = 0; /***** placeholder *******/
    bool quitGame = false;
    Game game; // Init game

    while (!WindowShouldClose() && !quitGame) {
        const float dt = GetFrameTime(); // Passed to game

        // Update
        //----------------------------------------------------------------------------------
        // Conditionally handle updates by screen as seen in example: https://www.raylib.com/examples/core/loader.html?name=core_basic_screen_manager
        switch (currentScreen) {
            case Screen::StartMenu: {
                // Handle User selection
                if (HandleMenuSelection(startMenuItems, selectedStartMenuItem, sounds)) {
                    switch (selectedStartMenuItem) {
                        case 0: {
                            if (game.IsGameOver())
                                game.NewGame();
                            currentScreen = Screen::Gameplay;
                            break;
                        }
                        case 1: {
                            currentScreen = Screen::HowToPlay;
                            break;
                        }
                        case 2: {
                            currentScreen = Screen::Options;
                            break;
                        }
                        case 3: {
                            selectedQuitPromptItem = 1;
                            PlayMapSound(sounds, "prompt");
                            currentScreen = Screen::QuitPrompt;
                            break;
                        }
                    }
                }
            } break;

            case Screen::Gameplay: {
                if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE)) {
                    currentScreen = Screen::StartMenu;
                    break;
                }

                // Handle input and update game
                const int prevScore = game.Score();
                const bool prevStarted = game.HasStarted();
                game.HandleInput();

                // If game has started, play sound:
                if (!prevStarted && game.HasStarted())
                    PlayMapSound(sounds, "start");

                game.Update(dt);

                // If the score went up after update, infer the snake eat and play sound
                if (game.Score() > prevScore)
                    PlayMapSound(sounds, "eat");

                // If game end, show Game Over screen
                if (game.IsGameOver()) {
                    if (game.GetGameEndType() == GameEndType::BoardFull)
                        PlayMapSound(sounds, "win");
                    else
                        PlayMapSound(sounds, "lose");

                    if (game.Score() > highScore)
                        highScore = game.Score();

                    currentScreen = Screen::GameOver;
                }
            } break;

            case Screen::HowToPlay: {
                if (HandleMenuSelection(howToPlayItems, selectedHowToPlayItem, sounds))
                    currentScreen = Screen::StartMenu;
            } break;

            case Screen::Options: {
                //*** Handle keyboard row navigation ***
                const int prevFocusedOptionRow = focusedOptionRow;

                // Upwards navigation with wraparound
                if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
                    focusedOptionRow = (focusedOptionRow - 1 + OPTIONS_COUNT) % OPTIONS_COUNT;
                }
                // Downwards navigation with wraparound
                if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
                    focusedOptionRow = (focusedOptionRow + 1) % OPTIONS_COUNT;
                }

                // Play sound if focus changed
                if (focusedOptionRow != prevFocusedOptionRow)
                    PlayMapSound(sounds, "move");

                //*** Handle keyboard selection ***
                // Allow confirm with Enter or Space
                const bool hasConfirm = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) || IsKeyPressed(KEY_SPACE);

                // Handle Sound Toggle
                if (UpdateToggleButton(soundToggle, focusedOptionRow == OPTIONS_SOUND && hasConfirm)) {
                    soundToggle.label = soundToggle.isToggled ? "ON" : "OFF"; // Change sound toggle label according to isToggled
                    focusedOptionRow = OPTIONS_SOUND;
                    PlayMapSound(sounds, "select");
                }

                // Handle Speed Toggle Group (left/right to navigate group buttons)
                int shift = 0;
                if (focusedOptionRow == OPTIONS_SPEED) {
                    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))
                        shift = -1;
                    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
                        shift = 1;
                }

                if (UpdateToggleGroup(speedToggleGroup, shift)) {
                    focusedOptionRow = OPTIONS_SPEED;
                    PlayMapSound(sounds, "select");
                }

                // Handle Back menu item select
                const bool backClicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
                    CheckCollisionPointRec(GetMousePosition(), optionsMenuItems[0].rect);
                // If Back menu item was clicked or selected by keyboard or ESC/BackSpace was keyed, go back to start menu.
                if ((focusedOptionRow == OPTIONS_BACK && hasConfirm) || backClicked ||
                    IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE)) {
                    PlayMapSound(sounds, "back");
                    focusedOptionRow = OPTIONS_SOUND; // Reset focus to first item
                    currentScreen = Screen::StartMenu;
                }
            } break;

            case Screen::GameOver: {
                if (HandleMenuSelection(gameOverItems, selectedGameOverItem, sounds)) {
                    if (selectedGameOverItem == 0) {
                        game.NewGame();
                        currentScreen = Screen::Gameplay;
                    } else {
                        selectedGameOverItem = 0;
                        currentScreen = Screen::StartMenu;
                    }
                }
            } break;

            case Screen::QuitPrompt: {
                if (HandleMenuSelection(quitPromptItems, selectedQuitPromptItem, sounds)) {
                    if (selectedQuitPromptItem == 0)
                        quitGame = true;
                    else
                        currentScreen = Screen::StartMenu;
                }
            } break;
        }

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        ClearBackground(BACKGROUND_COLOR); // Change background to dark blue

        // Conditionally render screen as seen in example: https://www.raylib.com/examples/core/loader.html?name=core_basic_screen_manager
        switch (currentScreen) {
            case Screen::StartMenu: {
                DrawStartMenuScreen(title, subTitle, startMenuItems, selectedStartMenuItem, keyTextures, howToFont);
            } break;

            case Screen::HowToPlay: {
                // Draw main How to Play Content
                DrawHowToPlayContent(howToFont);

                // Draw Back button
                DrawMenuItems(howToPlayItems, selectedHowToPlayItem);

                // Show Hint Bar
                DrawHintBar(HOW_TO_PLAY_HINT_LINE, keyTextures, howToFont);
            } break;

            case Screen::Options: {
                DrawCenteredText("Options", 60, 48, SCREEN_TITLE_COLOR);
                DrawCenteredText("Customize your gaming experience", 130, 20, LIGHTGRAY);

                // DrawText("Sound Effects", 80, static_cast<int>(soundToggle.rect.y + (soundToggle.rect.height - OPTIONS_HEADING_FONT_SIZE) / 2),
                //     OPTIONS_HEADING_FONT_SIZE, GOLD);
                DrawText("Sound Effects", 80, static_cast<int>(soundToggle.rect.y) - 20,
                    OPTIONS_HEADING_FONT_SIZE, GOLD);

                DrawTextLinesEx(80, soundToggle.rect.y + 14.0f, { "Toggle navigation and","gameplay sound effects." }, howToFont);

                DrawToggleButton(soundToggle, focusedOptionRow == OPTIONS_SOUND);

                // DrawText("Snake Speed", 80, static_cast<int>(speedToggleGroup.rect.y + (speedToggleGroup.rect.height - OPTIONS_HEADING_FONT_SIZE) / 2),
                DrawText("Snake Speed", 80, static_cast<int>(speedToggleGroup.rect.y) - 20,
                    OPTIONS_HEADING_FONT_SIZE, GOLD);
                DrawToggleGroup(speedToggleGroup, focusedOptionRow == OPTIONS_SPEED);
                DrawTextLinesEx(80, speedToggleGroup.rect.y + 14.0f, { "Set the starting snake", "speed." }, howToFont);


                // Handle mouse hover highlight on Back menu ite,m
                bool backHasHover = CheckCollisionPointRec(GetMousePosition(), optionsMenuItems[0].rect);
                const bool backHighlighted = focusedOptionRow == OPTIONS_BACK || backHasHover;
                DrawMenuItems(optionsMenuItems, backHighlighted ? 0 : -1);
            } break;

            case Screen::Gameplay: {
                game.Draw();

                // Show Hint Bar
                DrawHintBar(GAMEPLAY_HINT_LINE, keyTextures, howToFont);
            } break;

            case Screen::GameOver: {
                DrawCenteredText("GAME OVER", 100, 64, ORANGE, TextEffect::Blink);
                // Explain why the game ended
                DrawCenteredText(game.GetGameEndMessage(), 180, 24, LIGHTGRAY);

                // Draw Score
                DrawCenteredText(TextFormat("FINAL SCORE:   %d", game.Score()), 230, 32, LIGHTGRAY);
                //DrawCenteredText(TextFormat("FINAL SCORE: %d    High Score: %d", game.Score(), highScore), 184, 24, LIGHTGRAY);

                // Draw menu
                DrawMenuItems(gameOverItems, selectedGameOverItem);

                // Draw hint bar
                DrawHintBar(MENU_HINT_LINE, keyTextures, howToFont);
            } break;

            case Screen::QuitPrompt: {
                // Draw a dimmed start menu in the background
                DrawStartMenuScreen(title, subTitle, startMenuItems, selectedStartMenuItem, keyTextures, howToFont);
                DrawRectangle(0, 0, SCREEN_WIDTH, HINT_BAR_Y, Fade(BLACK, 0.6f));

                // Draw Quit Prompt
                const float roundness = 0.1f;
                const Rectangle dialog = { 200.0f, 150.0f, SCREEN_WIDTH / 2, 280.0f };
                DrawRectangleRounded(dialog, roundness, 8, QUIT_PROMPT_COLOR);
                DrawRectangleRoundedLines(dialog, roundness, 8, ORANGE);
                DrawCenteredText("Quit Game?", 176, 36, ORANGE);
                DrawCenteredText("The game window will close.", 230, 24, LIGHTGRAY);

                // Draw menu items
                DrawMenuItems(quitPromptItems, selectedQuitPromptItem);
            } break;
        }
        EndDrawing();
    }
    // Unload resources
    UnloadFont(howToFont);
    UnloadKeyTextures(keyTextures);
    UnloadSounds(sounds);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}

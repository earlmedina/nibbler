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
                if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE))
                    currentScreen = Screen::StartMenu;
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

#include <raylib.h>
#include <string>
#include <vector>
#include <cmath>
#include "constants.h"
#include "game.h"
#include "options.h"
#include "profile.h"
#include "profile_ui.h"
#include "resources.h"
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

    /*** Load local player profiles ***/
    const std::string profileDataPath = "profiles.json";
    ProfileStore profileStore = ProfileStore::Load(profileDataPath);

    // On first run with no profiles saved, prompt for initials, else skip to start menu
    Screen currentScreen = profileStore.profiles.empty()
        ? Screen::CreateProfile
        : Screen::StartMenu;

    InitialsScreen initialsScreen; // Declare InitialsScreen - default canCancel is false!
    std::vector<MenuItem> playerMenuItems; // Declare placeholder playerMenuItems
    int selectedPlayerMenuItem = 0; // Select first player profile

    float existingPlayerTimer = 0.0f; // Used to show/dismiss prompt when an existing player is entered

    // Get settings to use
    const PlayerProfile defaultSettings; // Declare empty profile which has default settings
    // If there are no profiles (which there should be at this point, so this is a failsafe),
    // the start profile uses default settings else the last used profile
    const PlayerProfile &startProfile = profileStore.profiles.empty()
        ? defaultSettings
        : profileStore.Current();

    /*** Options ***/
    // UPDATE WITH MICROSERVICES LATER!!!
    SetMasterVolume(startProfile.soundOn ? 1.0f : 0.0f); // Set master volume pased on user setting
    OptionsScreen options = CreateOptionsScreen(startProfile.soundOn, startProfile.speedSetting);

    /*** Game Vars ***/
    bool quitGame = false;
    Game game; // Init game

    // Declare lambda function to update settings if profile is changed
    // Use [&] to make sure outside options is updated
    // https://www.w3schools.com/cpp/cpp_functions_lambda.asp
    auto ApplySettingsFromProfile = [&](const PlayerProfile &p) {
        options = CreateOptionsScreen(p.soundOn, p.speedSetting); // Update options screen
        SetMasterVolume(p.soundOn ? 1.0f : 0.0f);
        game.NewGame(p.speedSetting);
    };

    /************ MENU SETUP ************/
    // Create array to store menu options and then set placement values

    /*** Start Menu ***/
    std::vector<MenuItem> startMenuItems = {
    { "Play", {} },
    { "How to Play", {} },
    { "Options", {} },
    { "Change Player", {} },
    { "Quit", {} }
    };

    // Set placement
    enum StartMenuOption { START_PLAY, START_HOW_TO, START_OPTIONS, START_CHANGE_PLAYER, START_QUIT }; // Use enum instead of numbers for clarity
    LayoutMenu(startMenuItems, 200.0f, 320.0f, 46.0f, 60.0f);
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
                        case START_PLAY: {
                            if (game.IsGameOver() || !game.HasStarted())
                                game.NewGame(profileStore.Current().speedSetting);
                            currentScreen = Screen::Gameplay;
                            break;
                        }
                        case START_HOW_TO: {
                            currentScreen = Screen::HowToPlay;
                            break;
                        }
                        case START_OPTIONS: {
                            currentScreen = Screen::Options;
                            break;
                        }
                        case START_CHANGE_PLAYER: {
                            playerMenuItems = LayoutPlayerMenu(profileStore); // Refresh player list
                            selectedPlayerMenuItem = profileStore.current; // Set selected as current profile
                            currentScreen = Screen::ChangePlayer;
                            break;
                        }
                        case START_QUIT: {
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

                    PlayerProfile &p = profileStore.Current(); // Get current profile
                    p.gamesPlayed++; // Increment games played

                    // Update player high score
                    if (game.Score() > p.highScore)
                        p.highScore = game.Score();

                    // Save profile
                    profileStore.Save(profileDataPath);

                    currentScreen = Screen::GameOver;
                }
            } break;

            case Screen::HowToPlay: {
                if (HandleMenuSelection(howToPlayItems, selectedHowToPlayItem, sounds))
                    currentScreen = Screen::StartMenu;
            } break;

            case Screen::Options: {
                if (UpdateOptionsScreen(options, sounds)) {
                    // If settings were updated:
                    PlayerProfile &p = profileStore.Current();
                    p.soundOn = options.soundToggle.isToggled;
                    p.speedSetting = options.speedGroup.selected;
                    profileStore.Save(profileDataPath);
                    currentScreen = Screen::StartMenu;
                }
            } break;

            case Screen::CreateProfile: {
                const InitialsResult result = UpdateInitialsScreen(initialsScreen, sounds);
                if (result == InitialsResult::Submitted) {
                    bool profileCreated = profileStore.SelectOrCreate(initialsScreen.text);
                    profileStore.Save(profileDataPath);
                    ApplySettingsFromProfile(profileStore.Current());
                    if (profileCreated) {
                        currentScreen = Screen::StartMenu;
                    } else {
                        existingPlayerTimer = EXISTING_PLAYER_MSG_DURATION;
                        PlayMapSound(sounds, "prompt");
                        currentScreen = Screen::ExistingPlayerPrompt;
                    }
                } else if (result == InitialsResult::Canceled) {
                    currentScreen = Screen::ChangePlayer;
                }
            } break;

            case Screen::ChangePlayer: {
                if (HandleMenuSelection(playerMenuItems, selectedPlayerMenuItem, sounds)) {
                    const int profileCount = static_cast<int>(profileStore.profiles.size());

                    // Handle profile switch
                    if (selectedPlayerMenuItem < profileCount) {
                        profileStore.current = selectedPlayerMenuItem;
                        profileStore.Save(profileDataPath); // Update last used profile
                        ApplySettingsFromProfile(profileStore.Current()); // Update settings from profile
                        currentScreen = Screen::StartMenu;
                    } else if (playerMenuItems[selectedPlayerMenuItem].isBack) {
                        currentScreen = Screen::StartMenu;
                    } else {
                        // If not back or existing profile, "Add New Player" was selected
                        initialsScreen = { "", true }; // reset text and make cancelable
                        currentScreen = Screen::CreateProfile;
                    }
                }
            } break;

            case Screen::ExistingPlayerPrompt: {
                // Manual dismiss
                if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER) ||
                    IsKeyPressed(KEY_KP_ENTER) || IsKeyPressed(KEY_SPACE)) {
                    currentScreen = Screen::StartMenu;
                    break;
                }

                // Auto dismiss logic
                existingPlayerTimer -= dt;
                if (existingPlayerTimer <= 0.0f)
                    currentScreen = Screen::StartMenu;
            } break;

            case Screen::GameOver: {
                if (HandleMenuSelection(gameOverItems, selectedGameOverItem, sounds)) {
                    if (selectedGameOverItem == 0) {
                        game.NewGame(profileStore.Current().speedSetting);
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
                DrawOptionsScreen(options, howToFont);
                // Draw hint bar
                DrawHintBar(OPTIONS_HINT_LINE, keyTextures, howToFont);
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

            case Screen::CreateProfile: {
                DrawInitialsScreen(initialsScreen, keyTextures, howToFont);
            } break;

            case Screen::ChangePlayer: {
                DrawCenteredText("Change Player", 60, 48, SCREEN_TITLE_COLOR);
                DrawCenteredText("Select a Player profile or add a new one.", 130, 24, LIGHTGRAY);
                DrawMenuItems(playerMenuItems, selectedPlayerMenuItem);

                // Draw hint bar
                DrawHintBar(MENU_HINT_LINE, keyTextures, howToFont);
            } break;

            case Screen::ExistingPlayerPrompt: {
                // Draw dimmed initials screen in background
                DrawInitialsScreen(initialsScreen, keyTextures, howToFont);
                DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.6f));

                // Draw prompt
                const float roundness = 0.1f;
                const Rectangle dialog = { 200.0f, 190.0f, SCREEN_WIDTH / 2.0f, 200.0f };
                DrawRectangleRounded(dialog, roundness, 9, QUIT_PROMPT_COLOR);
                DrawRectangleRoundedLines(dialog, roundness, 8, ORANGE);
                DrawCenteredText("Player Exists", 216, 36, ORANGE);
                DrawCenteredText(TextFormat("%s already exists.", initialsScreen.text.c_str()), 276, 24, LIGHTGRAY);
                DrawCenteredText("Switching to that player.", 310, 24, LIGHTGRAY);
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

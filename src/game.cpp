#include "game.h"
#include <cmath>
#include "raylib.h"
#include "ui.h"

Game::Game() {
    NewGame(); // Set up new game on init
}

/**
 * Sets up new game by resetting fields to default
 */
void Game::NewGame(int speedSetting) {
    snake.Reset(); // Reset snake position/size
    food.Spawn(snake); // Spawn food
    score = 0; // Reset score
    gameStarted = false; // Reset gameStarted flag
    gameOver = false; // Reset gameOver flag
    gameEndType = GameEndType::None; // Reset game end type
    stepInterval = STEP_INTERVALS[speedSetting];
    timeSinceStep = 0.0f; // Reset step time
}

/**
 * Queue snake turns from player input
 */
void Game::HandleInput() {
    bool directionKeyPressed = false;
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        snake.QueueTurn(Direction::Up);
        directionKeyPressed = true;
    }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        snake.QueueTurn(Direction::Down);
        directionKeyPressed = true;
    }
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
        snake.QueueTurn(Direction::Left);
        directionKeyPressed = true;
    }
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
        snake.QueueTurn(Direction::Right);
        directionKeyPressed = true;
    }
    if (directionKeyPressed && !gameStarted) {
        gameStarted = true;
        timeSinceStep = 0.0f;
    }
}

/**
 * Apply game logic by step
 * @param dt - Frame time
 */
void Game::Update(float dt) {
    if (gameOver || !gameStarted) return;

    timeSinceStep += dt; // Update step time
    while (timeSinceStep >= stepInterval && !gameOver) {
        timeSinceStep -= stepInterval; // Reset step time for next step
        Step();
    }
}

/**
 * Draw grid, food, snake, and score
 */
void Game::Draw() const {
    DrawGrid();
    snake.Draw();
    food.Draw();
    if (SHOW_LIVE_SCORE)
        DrawCenteredText(TextFormat("Score: %d", score), 10, 20, {255, 203, 0, 150});

    // Draw start text
    if (!gameStarted) {
        const int startPromptFontSize = 20;
        const char *startPromptText = "Press an arrow key or WASD to start";

        const float startY = 380.0f;
        const float startPromptHeight = 60.0f;
        const float startPromptWidth = 40 + MeasureText(startPromptText, startPromptFontSize);

        const Rectangle startPromptRect = {
            (SCREEN_WIDTH - startPromptWidth) / 2,
            startY, startPromptWidth, startPromptHeight
        };
        DrawRectangleRec(startPromptRect, Fade(BLACK, 0.6f));
        DrawRectangleLinesEx(startPromptRect, 1.0f, GOLD);

        const int textStartY = static_cast<int>(startY + (startPromptHeight - startPromptFontSize) / 2);
        DrawCenteredText(startPromptText, textStartY, startPromptFontSize, RAYWHITE);
    }
}

bool Game::IsGameOver() const {
    return gameOver;
}

GameEndType Game::GetGameEndType() const {
    return gameEndType;
}

bool Game::HasStarted() const {
    return gameStarted;
}

int Game::Score() const {
    return score;
}

const char *Game::GetGameEndMessage() const {
    switch (gameEndType) {
        case GameEndType::BoardFull: return "Congratulations! You filled the board!";
        case GameEndType::SelfCollision: return "You ran into yourself!";
        case GameEndType::WallCollision: return "You hit a wall!";
        case GameEndType::None: // Intentional fallthrough
        default: return ""; // Print nothing
    }
}

// From CodeLucky explanation:
// - The game runs a timer
// - 5-15 times every second the game updates = 0.05f - 0.15f --- smaller time = faster snake
// - In every interval, the snake moves one step
// https://www.youtube.com/watch?v=dZF0zseLcY0
void Game::Step() {
    // If the snake move results in a collision, end the game.
    MoveResult snakeMoveResult = snake.Move();
    if (snakeMoveResult != MoveResult::Ok) {
        gameEndType = snakeMoveResult == MoveResult::SelfCollision
            ? GameEndType::SelfCollision
            : GameEndType::WallCollision;
        gameOver = true;
        return;
    }

    // If the snake head is at the food position:
    // - Set the isGrowing flag to true.
    // - Increment the score.
    // - Slightly increase the snake's speed by decreasing the step interval.
    // - Spawn new food - if, in the unlikely case there are no free cells left, end the game.
    //   - Add a million points to the score as the player has "beaten" the game.
    if (snake.Head() == food.Position()) {
        if (ENABLE_GROWTH)
            snake.Grow();
        score++;
        stepInterval = fmaxf(MIN_STEP_INTERVAL, stepInterval - FOOD_SPEED_BOOST);
        if (!food.Spawn(snake)) {
            score += 1000000;
            gameEndType = GameEndType::BoardFull;
            gameOver = true;
        }
    }

}

/**
 * Draws alternating squares against background to produce a checkerboard pattern
 */
void Game::DrawGrid() {
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            // Draw alternating squares to create checker effect against background
            if ((r + c) % 2 == 0) {
                DrawRectangleRec(Cell{c, r}.ToRect(), ALTERNATE_COLOR);
            }
        }
    }
}

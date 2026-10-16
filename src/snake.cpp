#include <deque>

#include <algorithm>
#include "snake.h"
#include "cell.h"
#include "constants.h"
#include "raylib.h"


// Constructor
Snake::Snake() {
    // Snake should reset to default on init
    Reset();
}

/** Attributes **/
Cell Snake::Head() const { return body.front(); }
size_t Snake::Length() const { return body.size(); }

/**
 * Method used to return Snake attributes to default
 */
void Snake::Reset() {
    body.clear(); // Clear body deque
    turnQueue.clear(); // Clear any pending turns

    // Set snake start position
    const int startHeadX = COLS / 2 - 3;
    const int startHeadY = ROWS / 2;
    body.push_back({startHeadX, startHeadY});
    // Make the snake longer for sprint 1 demonstration purposes
    if (!ENABLE_GROWTH) {
        body.push_back({startHeadX - 1, startHeadY});
        body.push_back({startHeadX - 2, startHeadY});
    }
    currentDirection = Direction::Right;
    isGrowing = false;
}

/**
* Queue a turn which is checked against the turnQueue to determine whether it's valid:
*   - no reversing   - no same direction
 * @param newDirection - the next cell to move to
 */
void Snake::QueueTurn(const Cell newDirection) {
    // Get last direction - if the queue is empty it's the current direction otherwise the last move
    const Cell lastTurn = turnQueue.empty() ? currentDirection : turnQueue.back();

    // The last direction is the same or rerverse of the current one, do nothing.
    if (newDirection == lastTurn || (newDirection.x == -lastTurn.x && newDirection.y == -lastTurn.y))
        return;

    // All max 3 turns in queue: if under, then add the new direction
    if (turnQueue.size() < 3)
        turnQueue.push_back(newDirection);
}

/**
 * Perform a movement based on turnQueue or current direction (if empty)
 *  Note: used this CodeLogic video to get a general sense of how Snake movement logic
 *  should work in a Snake game: https://www.youtube.com/watch?v=dZF0zseLcY0
 * @return MoveResult enum value
 */
MoveResult Snake::Move() {
    // If the turnQueue is not empty, then set currentDirection to next queued direction and pop
    // (Otherwise, there's no queued turns and we simply move in the current direction
    if (!turnQueue.empty()) {
        currentDirection = turnQueue.front();
        turnQueue.pop_front();
    }

    // For movement, we determine what the new head will be
    const Cell nextHead = Head() + currentDirection;

    // If the next head is not within bounds (collides with wall) return WallCollision result
    if (!nextHead.WithinBounds())
        return MoveResult::WallCollision;

    /********* Collision Checks *********/
    // Body Collision:
    // For this check, we need to know what the next body size will be
    // - Case 1: The snake has not eaten, so we pop the tail and use body size minus 1
    // - Case 2: The snake has eaten, so the tail is not popped and the full body size is used
    const size_t newBodySize = isGrowing ? body.size() : body.size() - 1;

    // Check new head position against body
    for (size_t i = 0; i < newBodySize; i++) {
        if (body[i] == nextHead) {
            return MoveResult::SelfCollision; // Return SelfCollision if snake self collides
        }
    }

    // Otherwise, push next head to front
    body.push_front(nextHead);

    // And, pop tail if necessary
    if (isGrowing)
        isGrowing = false;
    else
        body.pop_back();

    return MoveResult::Ok; // return Ok for collisionless move
}

/**
 * Set isGrowing flag to true
 */
void Snake::Grow() {
    isGrowing = true;
}

/**
 * Helper function that checks if the snake's body contains a cell.
 * Used to determine where to spawn food.
 * Note: written as const member function because we don't want to modify the snake:
 * https://www.learncpp.com/cpp-tutorial/const-class-objects-and-const-member-functions/
 * @param cell - the Cell to check
 * @return true if the snake occupies the cell, false otherwise.
 */
bool Snake::Contains(Cell cell) const {
    // std::find "returns an iterator to the first element in the source range [first, last)
    // that satisfies specific criteria (or last if there is no such iterator)."
    // https://cppreference.com/cpp/algorithm/find
    return std::find(body.begin(), body.end(), cell) != body.end();
    // If the find result is body.end(), the snake doesn't contain the cell!
}

// Draw the snake
void Snake::Draw() const {
    for (size_t i = 0; i < body.size(); i++) {
        const Color color = i == 0 ? LIME : DARKGREEN;
        DrawRectangleRounded(body[i].ToRect(), 0.4f, 6, color);
    }
}

#pragma once

#include <deque>
#include "cell.h"
#include "constants.h"

class Snake {
public:
    Snake();

    /** Attributes **/
    Cell Head() const;
    size_t Length() const;

    void Reset();
    void QueueTurn(const Cell newDirection);
    MoveResult Move();
    void Grow();
    bool Contains(Cell cell) const;
    void Draw() const;

private:
    std::deque<Cell> body;
    std::deque<Cell> turnQueue;
    Cell currentDirection;
    bool isGrowing = false;
};

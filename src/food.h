#pragma once
#include "cell.h"

class Snake;

class Food {
public:
    Cell Position() const;
    bool Spawn(const Snake &snake);

    void Draw() const;

private:
    Cell position{};
};

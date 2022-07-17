#pragma once
#include "SuperSnake.hpp"

class Solver
{
public:

	virtual SuperSnake::SnakeAction solve(const SuperSnake::Game& game, SuperSnake::SnakeID id) = 0;

	virtual ~Solver() { }
};

using SolverGenerator = std::unique_ptr<Solver>(*)();

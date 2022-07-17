#pragma once
#include "Config.hpp"
#include "Solver.hpp"

class SolverRunner
{
public:

	struct SolverResult
	{
		size_t solverId;

		SuperSnake::SnakeID snakeId;

		int gameId;

		int step;

		std::future<SuperSnake::SnakeAction> future;
	};

	void solve(size_t solverId, const SuperSnake::Game& game, SuperSnake::SnakeID id);

	Optional<SolverResult> getResult(SuperSnake::SnakeID id);

	~SolverRunner();

private:

	struct SolverInstance
	{
		size_t solverId;

		SuperSnake::SnakeID snakeId;

		SuperSnake::Game gameCache;

		std::unique_ptr<Solver> solver;

		std::future<SuperSnake::SnakeAction> future;
	};

	std::list<SolverInstance> m_instance;

	static void solveImpl(Solver* solver, const SuperSnake::Game& game, SuperSnake::SnakeID id, std::promise<SuperSnake::SnakeAction> promise);
};

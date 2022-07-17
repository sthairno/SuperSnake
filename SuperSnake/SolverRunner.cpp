#include "SolverRunner.hpp"

void SolverRunner::solve(size_t solverId, const SuperSnake::Game& game, SuperSnake::SnakeID id)
{
	auto& instance = m_instance.emplace_back(SolverInstance{
		.solverId = solverId,
		.snakeId = id,
		.gameCache = game,
		.solver = std::move(Solvers[solverId].second())
	});

	std::packaged_task<SuperSnake::SnakeAction()> task([&] { return instance.solver->solve(instance.gameCache, instance.snakeId); });
	instance.future = task.get_future();
	
	std::thread thread(std::move(task));
	thread.detach();
}

Optional<SolverRunner::SolverResult> SolverRunner::getResult(SuperSnake::SnakeID id)
{
	auto itr = std::find_if(m_instance.begin(), m_instance.end(), [=](const SolverInstance& s) {
		return s.snakeId == id && s.future.wait_for(0s) == std::future_status::ready;
	});

	if (itr == m_instance.end())
	{
		return none;
	}

	SolverResult result{
		.solverId = itr->solverId,
		.snakeId = itr->snakeId,
		.gameId = itr->gameCache.gameId,
		.step = itr->gameCache.step(),
		.future = std::move(itr->future)
	};
	m_instance.erase(itr);

	return result;
}

SolverRunner::~SolverRunner()
{
	for (auto& instance : m_instance)
	{
		instance.future.wait();
	}
}

void SolverRunner::solveImpl(Solver* solver, const SuperSnake::Game& game, SuperSnake::SnakeID id, std::promise<SuperSnake::SnakeAction> promise)
{
	try
	{
		promise.set_value_at_thread_exit(solver->solve(game, id));
	}
	catch (std::exception)
	{
		promise.set_exception_at_thread_exit(std::current_exception());
	}
}

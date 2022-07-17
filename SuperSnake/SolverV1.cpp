#include "SolverV1.hpp"

using namespace SuperSnake;

SnakeAction SolverV1::solve(const Game& game, SnakeID id)
{
	m_game = &game;

	const auto& field = game.field();
	const auto fieldSize = field.size();
	const auto& snake = game.snakes()[id];
	m_bitField.resize(fieldSize.x * fieldSize.y);
	for (const Point pos : Iota2D(fieldSize))
	{
		m_bitField[pos.y * fieldSize.x + pos.x] = field[pos] != CellState::Unallocated;
	}

	// ハッシュテーブルを再生成
	if (m_fieldHashTable.size() != fieldSize.x * fieldSize.y)
	{
		m_fieldHashTable.resize(fieldSize.x * fieldSize.y);
		for (auto& hash : m_fieldHashTable)
		{
			hash = RandomInt32();
		}
	}

	m_pointHistory.clear();

	PointType maxPoint = 0;
	SnakeAction bestAction = SnakeAction::MoveStraight;
	for (int i : Range(-1, 1))
	{
		SnakeAction action = static_cast<SnakeAction>(i);
		PointType point = step(snake.position, snake.direction, 0, action, MaxStep);
		if (point > maxPoint)
		{
			bestAction = action;
			maxPoint = point;
		}
	}

	m_game = nullptr;

	return bestAction;
}

SolverV1::PointType SolverV1::step(Point currentPosition, Direction currentDirection, HashType currentFieldHash, SnakeAction action, int remainingStep)
{
	const auto nextDir = Direction((8 + int32(currentDirection) + int32(action)) % 8);
	const auto nextPos = currentPosition + Util::ToPoint(nextDir);
	const auto& field = m_game->field();

	if (nextPos.x < 0 ||
		nextPos.y < 0 ||
		nextPos.x >= field.size().x ||
		nextPos.y >= field.size().y)
	{
		return 0;
	}

	const int nextIdx = nextPos.y * field.size().x + nextPos.x;

	if (m_bitField[nextIdx])
	{
		return 0;
	}

	const HashType nextFieldHash = currentFieldHash ^ m_fieldHashTable[nextIdx];

	const auto history = m_pointHistory.find(nextFieldHash ^ m_directionHashTable[static_cast<int>(nextDir)]);
	if (history != m_pointHistory.cend())
	{
		return history->second;
	}

	m_bitField[nextIdx] = true;

	PointType totalPoint = 0;
	remainingStep--;
	if (remainingStep > 0)
	{
		for (int i : Range(-1, 1))
		{
			SnakeAction action = static_cast<SnakeAction>(i);
			PointType point = step(nextPos, nextDir, nextFieldHash, action, remainingStep);
			{
				totalPoint += point;
			}
		}
	}
	totalPoint++;

	m_bitField[nextIdx] = false;

	m_pointHistory.emplace(
		nextFieldHash ^ m_directionHashTable[static_cast<int>(nextDir)],
		totalPoint
	);

	return totalPoint;
}

std::unique_ptr<Solver> CreateSolverV1()
{
	return std::make_unique<SolverV1>();
}

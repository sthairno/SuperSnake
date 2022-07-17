#pragma once
#include "Solver.hpp"

class SolverV1 : public Solver
{
	using HashType = int32;

	using PointType = int64;

	constexpr static int MaxStep = 15;

public:

	SuperSnake::SnakeAction solve(const SuperSnake::Game& game, SuperSnake::SnakeID id) override;

private:

	// ZobristHash用ハッシュテーブル

	// フィールド用
	std::vector<HashType> m_fieldHashTable;
	// SnakeDirection用
	std::array<HashType, 8> m_directionHashTable{
		RandomInt32(),
		RandomInt32(),
		RandomInt32(),
		RandomInt32(),
		RandomInt32(),
		RandomInt32(),
		RandomInt32(),
		RandomInt32()
	};

	// ポイント履歴
	std::map<HashType, PointType> m_pointHistory;

	// フィールドをビットに置き換えた情報
	std::vector<bool> m_bitField;

	const SuperSnake::Game* m_game = nullptr;

	PointType step(Point currentPosition, SuperSnake::Direction currentDirection, HashType currentFieldHash, SuperSnake::SnakeAction action, int remainingStep);
};

std::unique_ptr<Solver> CreateSolverV1();

#pragma once
#include <Siv3D.hpp>

namespace SuperSnake
{
	using SnakeID = int32;

	enum class Direction
	{
		/// @brief 上 | ↑
		Up,
		/// @brief 右上 | ↗
		UpperRight,
		/// @brief 右 | →
		Right,
		/// @brief 右下 | ↘
		LowerRight,
		/// @brief 下 | ↓
		Down,
		/// @brief 左下 | ↙
		LowerLeft,
		/// @brief 左 | ←
		Left,
		/// @brief 左上 | ↖
		UpperLeft
	};

	enum class SnakeAction
	{
		/// @brief 左へ移動
		MoveLeft = -1,
		/// @brief 真っ直ぐ移動
		MoveStraight = 0,
		/// @brief 右へ移動
		MoveRight = 1,
		/// @brief 何もしない
		Stay = 2
	};

	enum class SnakeState
	{
		/// @brief 生存
		Alive,
		/// @brief 死亡(操作不能)
		Dead
	};

	struct Snake
	{
		/// @brief 獲得ポイント
		int point;

		/// @brief 名前
		String name;

		/// @brief 現在地
		Point position;

		/// @brief 向いている方向
		Direction direction;

		/// @brief 状態
		SnakeState state;

		/// @brief 胴体の座標(尾→頭)
		Array<Point> bodyPath;
	};

	enum class CellState
	{
		/// @brief 未確保
		Unallocated = -1,

		SnakeA = 0,
		SnakeB = 1,
		SnakeC = 2,
		SnakeD = 3,

		/// @brief 衝突
		Conflict = 4,
	};

	namespace Util
	{
		constexpr Point ToPoint(Direction direction)
		{
			switch (direction)
			{
			case Direction::Up:
				return { 0, -1 };
			case Direction::UpperRight:
				return { 1, -1 };
			case Direction::Right:
				return { 1, 0 };
			case Direction::LowerRight:
				return { 1, 1 };
			case Direction::Down:
				return { 0, 1 };
			case Direction::LowerLeft:
				return { -1, 1 };
			case Direction::Left:
				return { -1, 0 };
			case Direction::UpperLeft:
				return { -1, -1 };
			default:
				return { 0, 0 };
			}
		}

		constexpr Direction DoAction(Direction direction, SnakeAction action)
		{
			if (action == SnakeAction::Stay)
			{
				return direction;
			}
			return Direction((8 + int32(direction) + int32(action)) % 8);
		}
	}

	struct DeadEvent
	{
		SnakeID id;
	};

	struct GameOverEvent
	{
		Array<SnakeID> winnerList;
	};

	using GameEvent = std::variant<DeadEvent, GameOverEvent>;

	class Game
	{
	public:

		Game(Size fieldSize, int snakeCount, Array<Optional<String>> snakeNames = {});

		const int32 gameId;

		bool isGameOver() const { return m_gameOver; }

		int32 step() const { return m_step; }

		const Grid<CellState>& field() const { return m_field; }

		const Array<Snake>& snakes() const { return m_snakes; }

		Array<GameEvent> doActions(Array<SnakeAction> actions);

	private:

		int32 m_step = 0;

		bool m_gameOver = false;

		Grid<CellState> m_field;

		Array<Snake> m_snakes;
	};
}

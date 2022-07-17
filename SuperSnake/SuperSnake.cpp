#include "stdafx.h"
#include "SuperSnake.hpp"

namespace SuperSnake
{
	Game::Game(Size fieldSize, int snakeCount, Array<Optional<String>> snakeNames)
		: gameId(RandomInt32())
	{
		assert(fieldSize.x >= 2 && fieldSize.y >= 2);
		assert(1 <= snakeCount && snakeCount <= 4);

		m_field = Grid<CellState>(fieldSize, CellState::Unallocated);
		snakeNames.resize(snakeCount);
		for (SnakeID snakeID : Iota(snakeCount))
		{
			m_snakes.emplace_back(Snake{
				.point = 0,
				.name = snakeNames[snakeID]
				? *snakeNames[snakeID]
				: U"Snake {}"_fmt(char32_t(U'A' + snakeID)), // A ~ D
				.state = SnakeState::Alive,
				});

			// +------+
			// |0    2|
			// |      |
			// |3    1|
			// +------+
			Snake& snake = m_snakes.back();
			switch (snakeID)
			{
			case 0:
				snake.position = { 0, 0 };
				snake.direction = Direction::LowerRight;
				break;
			case 1:
				snake.position = { fieldSize.x - 1, fieldSize.y - 1 };
				snake.direction = Direction::UpperLeft;
				break;
			case 2:
				snake.position = { fieldSize.x - 1, 0 };
				snake.direction = Direction::LowerLeft;
				break;
			case 3:
				snake.position = { 0, fieldSize.y - 1 };
				snake.direction = Direction::UpperRight;
				break;
			}

			m_field[snake.position] = CellState(snakeID);
			snake.bodyPath.push_back(snake.position);
		}
	}

	Array<GameEvent> Game::doActions(Array<SnakeAction> actions)
	{
		assert(actions.size() == m_snakes.size());

		Array<GameEvent> events;

		if (m_gameOver)
		{
			return events;
		}

		std::array<bool, 4> moved;

		// 移動処理
		for (SnakeID snakeID : Iota(m_snakes.size()))
		{
			const SnakeAction action = actions[snakeID];
			Snake& snake = m_snakes[snakeID];

			if (snake.state == SnakeState::Alive &&
				action != SnakeAction::Stay)
			{
				snake.direction = Util::DoAction(snake.direction, action);
				snake.position += Util::ToPoint(snake.direction);
				snake.bodyPath.push_back(snake.position);
				moved[snakeID] = true;
			}
			else
			{
				moved[snakeID] = false;
			}
		}

		// フィールド更新処理, 衝突判定
		for (SnakeID snakeID : Iota(m_snakes.size()))
		{
			Snake& snake = m_snakes[snakeID];
			if (moved[snakeID])
			{
				// 領域判定
				if (not Rect{ 0, 0, m_field.size() }.contains(snake.position))
				{
					if (snake.state != SnakeState::Dead)
					{
						events.push_back(DeadEvent{ .id = snakeID });
					}
					snake.state = SnakeState::Dead;
					continue;
				}

				// 頭部衝突判定
				for (SnakeID otherSnakeID : Iota(snakeID + 1, m_snakes.size()))
				{
					Snake& otherSnake = m_snakes[otherSnakeID];
					if (otherSnake.position == snake.position)
					{
						if (otherSnake.state != SnakeState::Dead)
						{
							events.push_back(DeadEvent{ .id = otherSnakeID });
						}
						otherSnake.state = SnakeState::Dead;
						if (m_field[snake.position] == CellState::Unallocated)
						{
							m_field[snake.position] = CellState::Conflict;
						}
						break;
					}
				}

				// マス衝突判定
				if (m_field[snake.position] != CellState::Unallocated)
				{
					if (snake.state != SnakeState::Dead)
					{
						events.push_back(DeadEvent{ .id = snakeID });
					}
					snake.state = SnakeState::Dead;
					continue;
				}

				// マス確保, ポイント追加
				m_field[snake.position] = CellState(snakeID);
				snake.point++;
			}
		}

		// ゲームオーバー判定
		m_gameOver = m_snakes.all([](const Snake& s) { return s.state == SnakeState::Dead; });
		if (m_gameOver)
		{
			// 勝者判定, イベント発火
			int max = 0;
			Array<SnakeID> winnerList;
			for (SnakeID snakeID : Iota(m_snakes.size()))
			{
				Snake& snake = m_snakes[snakeID];
				if (snake.point > max)
				{
					winnerList = { snakeID };
				}
				else if (snake.point == max)
				{
					winnerList.push_back(snakeID);
				}
			}
			events.push_back(GameOverEvent{ .winnerList = winnerList });
		}

		m_step++;

		return events;
	}
}

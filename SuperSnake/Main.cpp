#include <Siv3D.hpp> // OpenSiv3D v0.6.4
#include "imgui.h"
#include "imgui_impl_s3d/imgui_impl_s3d.h"
#include "SuperSnake.hpp"
#include "Config.hpp"
#include "GameController.hpp"
#include "SettingsWindow.hpp"
#include "SolverRunner.hpp"
#include "KeyConfigWindow.hpp"

static double GetAxisValue(const detail::Gamepad_impl& gamepad, uint8 id)
{
	const uint8 idx = id / 2;
	if (idx >= gamepad.axes.size())
	{
		return 0.0;
	}
	else
	{
		const double mul = id % 2 ? 1.0 : -1.0;
		return gamepad.axes[idx] * mul;
	}
}

class App
{
public:

	App()
	{
		m_settings.startCallback = [this] { gameStart(); };
		m_settings.setVisible(true);
	}

	void update()
	{
		m_settings.renderWindow();
		if (not m_game || m_game->isGameOver())
		{
			m_footerText = U"";
		}
		else
		{
			if (std::all_of(
				m_indexedControllerStates.cbegin(),
				std::next(m_indexedControllerStates.cbegin(), m_game->snakes().size()),
				[this, id = 0](const std::shared_ptr<ControllerState>& state) mutable {
				return
					m_game->snakes()[id++].state == SuperSnake::SnakeState::Dead ||
					state->isDone;
			}))
			{
				if (m_nextStw.elapsed() > 1s)
				{
					m_next = true;
				}
			}
			else
			{
				m_nextStw.restart();
			}

			for (auto& state : m_controllerStates)
			{
				bool next = false;
				bool prev = false;
				Optional<int> pov;
				bool select = false;

				switch (state->controller.kind)
				{
				case GameController::Kind::Keyboard:
				{
					prev = KeyLeft.down();
					next = KeyRight.down();
					select = KeyEnter.down();

					if (KeyW.down()) pov = 0;
					else if (KeyE.down()) pov = 1;
					else if (KeyD.down()) pov = 2;
					else if (KeyC.down()) pov = 3;
					else if (KeyX.down()) pov = 4;
					else if (KeyZ.down()) pov = 5;
					else if (KeyA.down()) pov = 6;
					else if (KeyQ.down()) pov = 7;
				}
				break;
				case GameController::Kind::Gamepad:
				{
					if (auto& gamepad = Gamepad(state->controller.index))
					{
						auto config = GetKeyConfig(gamepad.getInfo());

						prev = config.leftButtonId < gamepad.buttons.size() && gamepad.buttons[config.leftButtonId].down();
						next = config.rightButtonId < gamepad.buttons.size() && gamepad.buttons[config.rightButtonId].down();
						select = config.selectButtonId < gamepad.buttons.size() && gamepad.buttons[config.selectButtonId].down();

						const Vec2 stick{ -GetAxisValue(gamepad, config.stickXaxisId), GetAxisValue(gamepad, config.stickYaxisId) };
						if (stick.length() > config.stickDeadzone)
						{
							pov = static_cast<int>(stick.getAngle() * 8 / Math::TwoPi + 4.5) % 8;
						}
						else if (JoyCon::IsJoyCon(gamepad))
						{
							pov = JoyCon(gamepad).povD8();
						}
					}
				}
				break;
				case GameController::Kind::Solver:
				{
					while (auto result = m_solverRunner.getResult(state->idList[state->targetIdx]))
					{
						const auto snakeId = result->snakeId;
						if (result->gameId == m_game->gameId &&
							result->step == m_game->step() &&
							m_game->snakes()[snakeId].state == SuperSnake::SnakeState::Alive)
						{
							try
							{
								m_actions[snakeId] = result->future.get();
								m_indexedControllerStates[snakeId]->isDone = true;
							}
							catch (std::exception ex)
							{
								Print << U"[Error] {}\n"_fmt(m_game->snakes()[snakeId].name) << Unicode::FromUTF8(ex.what());
							}
							catch (Error ex)
							{
								Print << U"[Error] {}\n"_fmt(m_game->snakes()[snakeId].name) << ex;
							}
						}
					}
				}
				}

				state->targetIdx +=
					next ? -1
					: prev ? 1
					: 0;
				state->targetIdx = (state->idList.size() + state->targetIdx) % state->idList.size();

				const int snakeId = state->idList[state->targetIdx];
				auto& snake = m_game->snakes()[snakeId];

				if (not state->isDone && pov)
				{
					auto direction = static_cast<SuperSnake::Direction>(*pov);
					for (int i : Range(-1, 1))
					{
						auto action = static_cast<SuperSnake::SnakeAction>(i);
						if (direction == SuperSnake::Util::DoAction(snake.direction, action))
						{
							m_actions[snakeId] = action;
						}
					}
				}

				if (select &&
					std::all_of(
						state->idList.cbegin(),
						state->idList.cend(),
						[this](int id) {
							return
								m_game->snakes()[id].state == SuperSnake::SnakeState::Dead ||
								m_actions[id] != SuperSnake::SnakeAction::Stay;
						}))
				{
					state->isDone = !state->isDone;
				}
			}
		}

		if (m_next)
		{
			nextStep();
			m_next = false;
		}
	}

	void draw(RectF rect)
	{
		const RectF baseRect = rect.stretched(-10);
		const SizeF nextButtonSize = SimpleGUI::ButtonRegion(U"Next▶", {}).size;
		const double headerHeight = nextButtonSize.y;
		const double footerHeight = m_font(m_footerText).region().h;

		const RectF headerRect{
			baseRect.pos,
			baseRect.w,
			headerHeight
		};

		const RectF contentRect{
			baseRect.x,
			baseRect.y + headerHeight,
			baseRect.w,
			baseRect.h - headerHeight - footerHeight
		};

		const RectF footerRect{
			baseRect.x,
			baseRect.y + baseRect.h - footerHeight,
			baseRect.w,
			footerHeight
		};

		const double fieldWidth = contentRect.w / 5 * 3;
		const double playerStateWidth = contentRect.w / 5;
		const double playerStateHeight = playerStateWidth * 0.8 + 40 + 16;

		const RectF fieldRect{
			contentRect.x + (contentRect.w - fieldWidth) / 2,
			contentRect.y,
			fieldWidth,
			contentRect.h
		};

		std::array<RectF, 4> playerStateRectList{
			RectF(Arg::topRight = fieldRect.tl(), playerStateWidth, playerStateHeight),
			RectF(Arg::bottomLeft = fieldRect.br(), playerStateWidth, playerStateHeight),
			RectF(Arg::topLeft = fieldRect.tr(), playerStateWidth, playerStateHeight),
			RectF(Arg::bottomRight = fieldRect.bl(), playerStateWidth, playerStateHeight),
		};

		//headerRect.drawFrame(1, 0, Palette::Blue);
		//contentRect.drawFrame(1, 0, Palette::Blue);
		//footerRect.drawFrame(1, 0, Palette::Blue);
		//fieldRect.drawFrame(1, 0, Palette::Green);
		//for (const RectF& rect : playerStateRectList)
		//{
		//	rect.drawFrame(1, 0, Palette::Red);
		//}

		m_font(m_footerText).draw(Arg::bottomCenter = footerRect.bottomCenter(), Palette::Gray);
		if (m_game)
		{
			if (not m_game->isGameOver() &&
				SimpleGUI::Button(U"Next▶", headerRect.tr() - Vec2{ nextButtonSize.x, 0 }))
			{
				m_next = true;
			}
			drawField(fieldRect);
			for (const SuperSnake::SnakeID id : Iota(m_game->snakes().size()))
			{
				drawPlayerState(playerStateRectList[id].stretched(-8 - 40, -8, -8, -8), id);
			}
		}
	}

private:

	Font m_font{ 24 };

	String m_stateText;

	String m_footerText;

	std::unique_ptr<SuperSnake::Game> m_game;

	SettingsWindow m_settings;

	Stopwatch m_nextStw;

	bool m_next = false;

	std::map<GameController::Kind, Texture> m_controllerTextures{
		{GameController::Kind::Network, Texture{ {Icon::Type::MaterialDesign, 0xF0317 }, 36 }},
		{GameController::Kind::Solver, Texture{ {Icon::Type::MaterialDesign, 0xF06A9 }, 36 }},
		{GameController::Kind::Gamepad, Texture{ {Icon::Type::MaterialDesign, 0xF02B4 }, 36 }},
		{GameController::Kind::Keyboard, Texture{ {Icon::Type::MaterialDesign, 0xF030C }, 36 }}
	};

	std::array<SuperSnake::SnakeAction, 4> m_actions = {
		SuperSnake::SnakeAction::Stay,
		SuperSnake::SnakeAction::Stay,
		SuperSnake::SnakeAction::Stay,
		SuperSnake::SnakeAction::Stay,
	};

	SolverRunner m_solverRunner;

	struct ControllerState
	{
		GameController controller;

		std::vector<int> idList;

		int targetIdx;

		bool isDone;
	};

	std::vector<std::shared_ptr<ControllerState>> m_controllerStates;

	std::array<std::shared_ptr<ControllerState>, 4> m_indexedControllerStates;

	void drawPlayerState(RectF rect, const SuperSnake::SnakeID id) const
	{
		const auto& snake = m_game->snakes()[id];
		const auto& action = m_actions[id];
		const ColorF frameColor = SnakeColors[id].lerp(Palette::Black, 0.1);
		const auto& controller = m_settings.selectedControllers()[id];
		const auto& controllerState = *m_indexedControllerStates[id];

		rect.rounded(PlayerStateBoxRound)
			.draw(Palette::White)
			.drawFrame(0, 4, frameColor);
		m_font(snake.name)
			.draw(Arg::topCenter = rect.topCenter(), Palette::Black);

		if (not controllerState.isDone && snake.state == SuperSnake::SnakeState::Alive)
		{
			const auto selectedId = controllerState.idList[controllerState.targetIdx];
			if (selectedId == id)
			{
				drawController(rect.tl() + Vec2{ 0, -8 }, controller);
			}
		}

		if (snake.state == SuperSnake::SnakeState::Dead)
		{
			const RectF contentRect = rect.stretched(-m_font.height(), 0, 0, 0);
			const double contentSize = Min(contentRect.w, contentRect.h) / 1.4;
			Shape2D::Cross(contentSize / 2, contentSize / 6, contentRect.center())
				.draw(Color(0xFF, 0x72, 0x72));
		}
		else
		{
			const RectF contentRect = rect.stretched(-m_font.height(), 0, -m_font.height(), 0);
			const double contentSize = Min(contentRect.w, contentRect.h) / 1.4;
			if (action == SuperSnake::SnakeAction::Stay)
			{
				RectF(Arg::center = contentRect.center(), contentSize * 0.8, contentSize * 0.14).draw(Palette::Lightslategray);
			}
			else
			{
				drawArrow(
					contentRect.center(),
					contentSize,
					SuperSnake::Util::DoAction(snake.direction, action),
					SnakeColors[id]);
			}

			const auto suggestionText = m_font(U"候補: ");
			const double suggestionWidth = suggestionText.region().size.x + m_font.height() * 3;

			Vec2 nextPos = rect.bl() + Vec2{ (rect.w - suggestionWidth) / 2, -m_font.height() };
			nextPos.x += suggestionText.draw(nextPos, ColorF{ 0.4 }).w;
			for (auto diff : Range(-1, 1))
			{
				SuperSnake::Direction suggestDirection =
					SuperSnake::Util::DoAction(snake.direction, static_cast<SuperSnake::SnakeAction>(diff));
				drawArrow(nextPos + Vec2{ m_font.height(), m_font.height() } / 2, m_font.height() - 4, suggestDirection, Palette::Gray);
				nextPos.x += m_font.height();
			}
		}
	}

	void drawField(RectF rect) const
	{
		const Size fieldSize = m_game->field().size();
		const double cellSize = Min(rect.w / (fieldSize.x + 2), rect.h / (fieldSize.y + 2));
		const RectF renderRect{ Arg::center = rect.center(), cellSize * fieldSize };
		const Mat3x2 renderMat(
			cellSize, 0.f,
			0.f, cellSize,
			renderRect.x, renderRect.y
		);

		renderRect.drawShadow({ 0, FrameThickness / 2 }, 8, FrameThickness);

		// Cell
		for (Point pos : Iota2D(fieldSize))
		{
			const auto cell = m_game->field()[pos];
			RectF cellRect{
				renderMat.transformPoint(pos),
				cellSize, cellSize
			};

			ColorF color;
			switch (cell)
			{
			case SuperSnake::CellState::Unallocated:
				color = DefaultCellColor;
				break;
			case SuperSnake::CellState::Conflict:
				color = ConflictCellColor;
				break;
			default:
				const auto id = SuperSnake::SnakeID(cell);
				color = SnakeColors[id].lerp(DefaultCellColor, 0.4);
				break;
			}

			cellRect.draw(color);
		}

		// Grid
		renderRect.drawFrame(FrameThickness, FrameColor);
		for (int y : Iota(1, fieldSize.y))
		{
			Line(
				renderMat.transformPoint(Point{ 0, y }),
				renderMat.transformPoint(Point{ fieldSize.x, y }))
				.draw(FrameThickness, FrameColor);
		}
		for (int x : Iota(1, fieldSize.x))
		{
			Line(
				renderMat.transformPoint(Point{ x, 0 }),
				renderMat.transformPoint(Point{ x, fieldSize.y }))
				.draw(FrameThickness, FrameColor);
		}

		// Body
		for (auto [snakeID, snake] : IndexedRef(m_game->snakes()))
		{
			LineString lineStr(Arg::reserve = snake.bodyPath.size());
			for (Point p : snake.bodyPath)
			{
				lineStr.push_back(renderMat.transformPoint(p + Vec2{ 0.5, 0.5 }));
			}
			lineStr.draw(LineStyle::RoundCap, cellSize * 0.4, SnakeColors[snakeID]);
		}

		// Head
		for (auto [snakeID, snake] : IndexedRef(m_game->snakes()))
		{
			ColorF color = snake.state == SuperSnake::SnakeState::Dead
				? DeadSnakeColor
				: SnakeColors[snakeID];
			Circle(renderMat.transformPoint(snake.position + Vec2{ 0.5, 0.5 }), cellSize * 0.4)
				.draw(color);
		}
	}

	void drawController(const Vec2 bottomLeft, const GameController& controller) const
	{
		const Texture& tex = m_controllerTextures.at(controller.kind);
		const double rectSize = Max({ tex.width(), tex.height(), m_font.height() }) * 1.1;
		const String str = controller.kind == GameController::Kind::Gamepad
			? Format(controller.index)
			: U"";

		RectF rect(Arg::bottomLeft = bottomLeft, rectSize + 4, rectSize);
		if (not str.empty())
		{
			rect.w += m_font(str).region().w + 4;
		}
		rect.rounded(rectSize / 4).draw(ColorF{ 0.4 });
		tex.drawAt(rect.leftCenter() + Vec2{ rectSize / 2 + 2, 0 }, Palette::White);
		m_font(str).draw(Arg::leftCenter = rect.leftCenter() + Vec2{ rectSize + 4, 0 }, Palette::White);
	}

	void gameStart()
	{
		m_game = std::make_unique<SuperSnake::Game>(
			m_settings.fieldSize(),
			m_settings.snakeCount(),
			m_settings.selectedControllers().map([](const GameController c) -> Optional<String> {
				switch (c.kind)
				{
				case GameController::Kind::Keyboard: return U"Keyboard";
				case GameController::Kind::Gamepad: return Gamepad(c.index).getInfo().name;
				case GameController::Kind::Solver: return Solvers[c.index].first;
				default: return none;
				}
				}));

		m_controllerStates.clear();
		m_indexedControllerStates.fill(nullptr);
		for (const auto [idx, controller] : Indexed(m_settings.selectedControllers()))
		{
			if (controller.kind == GameController::Kind::Keyboard ||
				controller.kind == GameController::Kind::Gamepad)
			{
				m_footerText = U"←/→ 操作対象選択, W/E/D/C/X/Z/A/Q 方向選択, [Enter] 確定\n[L]/[R] 操作対象選択, [Stick] 方向選択, [Select] 確定";
			}

			if (controller.kind == GameController::Kind::Keyboard ||
				controller.kind == GameController::Kind::Gamepad)
			{
				auto itr = std::find_if(
					m_controllerStates.begin(),
					m_controllerStates.end(),
					[&](std::shared_ptr<ControllerState>& s) {
						return s->controller == controller;
				});
				if (itr == m_controllerStates.end())
				{
					m_controllerStates.emplace_back(std::shared_ptr<ControllerState>(new ControllerState{
						.controller = controller,
						.idList = { int(idx) },
						.targetIdx = 0,
						}));
					m_indexedControllerStates[idx] = m_controllerStates.back();
				}
				else
				{
					auto state = *itr;
					state->idList.push_back(int(idx));
					std::sort(state->idList.begin(), state->idList.end());
					m_indexedControllerStates[idx] = state;
				}
			}
			else
			{
				m_controllerStates.emplace_back(std::shared_ptr<ControllerState>(new ControllerState{
					.controller = controller,
					.idList = { int(idx) },
					.targetIdx = 0,
					}));
				m_indexedControllerStates[idx] = m_controllerStates.back();
			}
		}
		beginStep();
	}

	void nextStep()
	{
		m_game->doActions(Array<SuperSnake::SnakeAction>(
			m_actions.cbegin(),
			std::next(m_actions.cbegin(), m_game->snakes().size())
			));
		if (not m_game->isGameOver())
		{
			beginStep();
		}
	}

	void beginStep()
	{
		for (auto& state : m_controllerStates)
		{
			state->isDone = false;
		}
		m_nextStw.restart();
		m_actions.fill(SuperSnake::SnakeAction::Stay);
		for (auto [idx, controller] : Indexed(m_settings.selectedControllers()))
		{
			if (controller.kind == GameController::Kind::Solver &&
				m_game->snakes()[idx].state == SuperSnake::SnakeState::Alive)
			{
				m_solverRunner.solve(controller.index, *m_game, idx);
			}
		}
	}

	static void drawArrow(Vec2 center, double size, SuperSnake::Direction direction, ColorF color)
	{
		int32 directionIdx = (int32)direction;

		Line arrowLine;
		arrowLine.end = Circular(size / 2, Math::TwoPi * directionIdx / 8);
		arrowLine.begin = center - arrowLine.end;
		arrowLine.end += center;
		Shape2D::Arrow(
			arrowLine,
			arrowLine.length() / 2.5,
			{ arrowLine.length() / 2, arrowLine.length() / 2 })
			.draw(color);
	}
};

void Main()
{
	LoadConfig();

	Window::SetTitle(U"SuperSnake");

	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_Impls3d_Init();

	Window::SetStyle(WindowStyle::Sizable);
	Window::SetToggleFullscreenEnabled(false);
	Scene::SetBackground(ColorF(0.9));

	bool fullScreen = false;
	App app;

	while (System::Update())
	{
		ImGui_Impls3d_NewFrame();
		ImGui::NewFrame();

		if (KeyF11.down())
		{
			fullScreen = not fullScreen;
			Window::SetFullscreen(fullScreen);
		}
		app.update();
		app.draw(Scene::Rect());

		ImGui::Render();
		ImGui_Impls3d_RenderDrawData(ImGui::GetDrawData());
	}
	ImGui_Impls3d_Shutdown();
}

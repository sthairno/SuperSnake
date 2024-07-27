#pragma once
#include "Solver.hpp"
#include "SolverV1.hpp"
#include "GameController.hpp"
#include "KeyConfig.hpp"
#include "GameSettings.hpp"

constexpr StringView ConfigPath = U"./config.bin";

constexpr ColorF DefaultCellColor = Palette::White;
constexpr ColorF ConflictCellColor = ColorF{ 0.7 };
constexpr double FrameThickness = 4;
constexpr ColorF FrameColor = ColorF{ 0.6 };
constexpr std::array<ColorF, 4> SnakeColors{
	Color(0x37, 0xDC, 0x94), // #37DC94
	Color(0x26, 0x8A, 0xFF), // #268AFF
	Color(0xFA, 0x5C, 0x65), // #FA5C65
	Color(0xFD, 0x9A, 0x28), // #FD9A28
};
constexpr ColorF DeadSnakeColor = ColorF{ 0.5 };
constexpr ColorF ActionConfirmedColor = Color{ 0x60, 0xD6, 0x66 }; // #60D666
constexpr SizeF PlayerStateBoxSize = { 220, 160 };
constexpr double PlayerStateBoxRound = 6;
constexpr double PlayerStateBoxThickness = 4;

constexpr std::array<std::pair<const char32_t*, SolverGenerator>, 1> Solvers{
	std::pair<const char32_t*, SolverGenerator>{U"SolverV1", CreateSolverV1}
};

KeyConfig GetKeyConfig(const GamepadInfo& info);

void SetKeyConfig(const GamepadInfo& info, KeyConfig config);

const HashTable<String, GameSettings>& GetGamePresets();

void SetGamePresets(const HashTable<String, GameSettings>& presets);

void LoadConfig();

void SaveConfig();

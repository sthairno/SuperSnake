#pragma once
#include "Solver.hpp"
#include "SolverV1.hpp"

const String ConfigPath = U"./config.bin";

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
constexpr SizeF PlayerStateBoxSize = { 220, 160 };
constexpr double PlayerStateBoxRound = 6;
constexpr double PlayerStateBoxThickness = 4;

constexpr std::array<std::pair<const char32_t*, SolverGenerator>, 1> Solvers{
	std::pair<const char32_t*, SolverGenerator>{U"SolverV1", CreateSolverV1}
};

struct KeyConfig
{
	uint8 selectButtonId = 0;

	uint8 cancelButtonId = 1;

	uint8 leftButtonId = 2;

	uint8 rightButtonId = 7;

	// even -> +, odd -> -

	uint8 stickXaxisId = 1;

	uint8 stickYaxisId = 2;

	double stickDeadzone = 0.5;
};

template<class Archive>
void SIV3D_SERIALIZE(Archive& archive, KeyConfig& config)
{
	archive(
		cereal::make_nvp("selectButtonId", config.selectButtonId),
		cereal::make_nvp("cancelButtonId", config.cancelButtonId),
		cereal::make_nvp("leftButtonId", config.leftButtonId),
		cereal::make_nvp("rightButtonId", config.rightButtonId),
		cereal::make_nvp("leftButtonId", config.leftButtonId),
		cereal::make_nvp("stickXaxisId", config.stickXaxisId),
		cereal::make_nvp("stickDeadzone", config.stickDeadzone)
	);
}

KeyConfig GetKeyConfig(const GamepadInfo& info);

void SetKeyConfig(const GamepadInfo& info, KeyConfig config);

void LoadConfig();

void SaveConfig();

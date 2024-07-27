#pragma once
#include <Siv3D.hpp>

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
static void SIV3D_SERIALIZE(Archive& archive, KeyConfig& config)
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

#pragma once
#include <Siv3D.hpp>
#include "GameController.hpp"

struct GameSettings
{
	Size fieldSize = { 10, 10 };

	Array<GameController> selectedControllers;
};

template<class Archive>
static void SIV3D_SERIALIZE(Archive& archive, GameSettings& settings)
{
	archive(
		cereal::make_nvp("fieldSize", settings.fieldSize),
		cereal::make_nvp("selectedControllers", settings.selectedControllers)
	);
}

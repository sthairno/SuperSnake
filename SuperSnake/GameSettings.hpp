#pragma once
#include <Siv3D.hpp>
#include "GameController.hpp"

struct GameSettings
{
	Size fieldSize = { 10, 10 };

	Array<GameController> selectedControllers{ 4, GameController::Unselected() };

	size_t snakeCount() const
	{
		return selectedControllers.size();
	}
};

template<class Archive>
static void SIV3D_SERIALIZE(Archive& archive, GameSettings& settings)
{
	archive(
		cereal::make_nvp("fieldSize", settings.fieldSize),
		cereal::make_nvp("selectedControllers", settings.selectedControllers)
	);
}

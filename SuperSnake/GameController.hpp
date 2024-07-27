#pragma once

struct GameController
{
	enum class Kind
	{
		Unselected,
		Network,
		Solver,
		Gamepad,
		Keyboard,
	};

	Kind kind;

	uint32 index;

	uint64 gamepadUid = 0;

	static GameController Unselected()
	{
		return { Kind::Unselected };
	}

	static GameController FromGamepadInfo(const GamepadInfo& info)
	{
		return { Kind::Gamepad, info.playerIndex, (static_cast<uint64>(info.vendorID) << 32) | info.productID };
	}
};

template<class Archive>
static void SIV3D_SERIALIZE(Archive& archive, GameController& controller)
{
	archive(
		cereal::make_nvp("kind", controller.kind),
		cereal::make_nvp("index", controller.index),
		cereal::make_nvp("gamepadUid", controller.gamepadUid)
	);
}

inline constexpr bool operator==(const GameController& a, const GameController& b)
{
	return a.kind == b.kind && a.index == b.index;
}

inline constexpr bool operator<(const GameController& a, const GameController& b)
{
	if (a.kind < b.kind)
	{
		return true;
	}
	else if (a.kind == b.kind)
	{
		return a.index < b.index;
	}
	return false;
}

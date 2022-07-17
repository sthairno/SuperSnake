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
};

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

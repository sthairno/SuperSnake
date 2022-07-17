#pragma once
#include "Config.hpp"

class KeyConfigWindow
{
public:
	
	KeyConfigWindow(const GamepadInfo& info)
		: target(info)
	{ }

	const GamepadInfo target;

	void setFocus(bool focus = true) { m_setFocus = focus; }

	void renderWindow(bool& visible);

private:

	int m_selectedIdx = -1;

	bool m_setFocus = false;
};

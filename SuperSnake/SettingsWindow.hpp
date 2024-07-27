#pragma once
#include "Config.hpp"
#include "GameController.hpp"
#include "KeyConfigWindow.hpp"

class SettingsWindow
{
public:

	std::function<void()> startCallback;

	const GameSettings& settings() const { return m_settings; }

	void setVisible(bool v) { m_visible = v; }

	void renderWindow();

private:

	static bool isConnected(const GameController controller);

	bool m_visible = false;

	GameSettings m_settings;

	std::map<GameController, std::string> m_controllerList;

	std::list<KeyConfigWindow> m_keyConfigWindows;

	std::string m_presetName;

	void renderControllerPicker(GameController& controller);
};

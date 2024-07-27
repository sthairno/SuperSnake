#pragma once
#include "Config.hpp"
#include "GameController.hpp"
#include "KeyConfigWindow.hpp"

class SettingsWindow
{
public:

	std::function<void()> startCallback;

	Size fieldSize() const { return m_fieldSize; }

	int snakeCount() const { return m_selectedControllers.size(); }

	const Array<GameController>& selectedControllers() const { return m_selectedControllers; }

	void setVisible(bool v) { m_visible = v; }

	void renderWindow();

private:

	static bool isConnected(const GameController controller);

	bool m_visible = false;

	Size m_fieldSize = { 10, 10 };

	Array<GameController> m_selectedControllers{ 4, GameController{.kind = GameController::Kind::Unselected } };

	std::map<GameController, std::string> m_controllerList;

	std::list<KeyConfigWindow> m_keyConfigWindows;

	std::string m_presetName;

	void renderControllerPicker(GameController& controller);
};

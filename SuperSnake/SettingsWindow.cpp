#include "SettingsWindow.hpp"
#include <imgui.h>

void SettingsWindow::renderWindow()
{
	if (not m_visible)
	{
		return;
	}

	m_controllerList = {
		{ GameController{ .kind = GameController::Kind::Unselected }, "---" },
		{ GameController{ .kind = GameController::Kind::Network }, "Network" },
		{ GameController{ .kind = GameController::Kind::Keyboard }, "Keyboard" }
	};
	for (auto [idx, pair] : Indexed(Solvers))
	{
		m_controllerList.emplace(
			GameController{ .kind = GameController::Kind::Solver, .index = static_cast<uint32>(idx) },
			fmt::format("Solver: {}##{}", String(pair.first).toUTF8(), idx)
		);
	}
	for (auto& gamepad : System::EnumerateGamepads())
	{
		m_controllerList.emplace(
			GameController{ .kind = GameController::Kind::Gamepad, .index = gamepad.playerIndex },
			fmt::format("Gamepad: {}##{}", gamepad.name.toUTF8(), gamepad.vendorID ^ gamepad.productID)
		);
	}

	ImGui::SetNextWindowSize({ 400, 280 });
	ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	ImGui::Begin("Settings", nullptr,
		ImGuiWindowFlags_::ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_::ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse);
	{
		ImGui::BulletText("Field");
		ImGui::Indent();
		{
			if (ImGui::InputInt("width", &m_fieldSize.x))
			{
				m_fieldSize.x = Max(m_fieldSize.x, 2);
			}
			if (ImGui::InputInt("height", &m_fieldSize.y))
			{
				m_fieldSize.y = Max(m_fieldSize.y, 2);
			}
		}
		ImGui::Unindent();

		ImGui::BulletText("Snake");
		ImGui::Indent();
		{
			if (ImGui::InputInt("count", &m_snakeCount))
			{
				m_snakeCount = Clamp(m_snakeCount, 1, 4);
				m_selectedControllers.resize(m_snakeCount, GameController{ .kind = GameController::Kind::Unselected });
			}
		}
		ImGui::Unindent();

		ImGui::BulletText("Controllers");
		ImGui::Indent();
		{
			for (int i : Iota(m_snakeCount))
			{
				auto& controller = m_selectedControllers[i];
				ImGui::Text("%d:", i);
				ImGui::SameLine();
				ImGui::PushID(i);
				{
					renderControllerPicker(controller);
					if (controller.kind == GameController::Kind::Gamepad)
					{
						ImGui::SameLine();

						auto& gamepad = Gamepad(controller.index);
						auto& gamepadInfo = gamepad.getInfo();

						if (ImGui::Button("Config"))
						{
							auto itr = std::find_if(m_keyConfigWindows.begin(), m_keyConfigWindows.end(), [&](const KeyConfigWindow& w) {
								return
									w.target.vendorID == gamepadInfo.vendorID &&
									w.target.productID == gamepadInfo.productID;
							});
							if (itr == m_keyConfigWindows.end())
							{
								m_keyConfigWindows.emplace_back(KeyConfigWindow(gamepadInfo));
							}
							else
							{
								itr->setFocus();
							}
						}

						if (gamepad.isConnected())
						{
							if (gamepad.buttons.includes_if([](const Input& i) {
								return i.pressed();
								}))
							{
								ImGui::SameLine();
								ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "*");
							}
						}
						else
						{
							ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "x");
						}
					}
				}
				ImGui::PopID();
			}
		}
		ImGui::Unindent();

		ImGui::BeginDisabled(m_selectedControllers.includes_if([](const GameController c) {
			return c.kind == GameController::Kind::Unselected;
			}));
		{
			if (ImGui::Button("Start"))
			{
				m_visible = false;
				if (startCallback)
				{
					startCallback();
				}
			}
		}
		ImGui::EndDisabled();
	}
	ImGui::End();

	for (auto itr = m_keyConfigWindows.begin(); itr != m_keyConfigWindows.end();)
	{
		bool visible = true;

		itr->renderWindow(visible);

		if (visible)
		{
			itr++;
		}
		else
		{
			itr = m_keyConfigWindows.erase(itr);
		}
	}
}

bool SettingsWindow::isConnected(const GameController controller)
{
	switch (controller.kind)
	{
	case GameController::Kind::Network: return true;
	case GameController::Kind::Gamepad: return Gamepad(controller.index).isConnected();
	default: return true;
	}
}

void SettingsWindow::renderControllerPicker(GameController& controller)
{
	if (not m_controllerList.contains(controller))
	{
		controller = GameController{ .kind = GameController::Kind::Unselected };
	}
	if (ImGui::BeginCombo("##ControllerPicker", m_controllerList[controller].data()))
	{
		for (auto& [id, str] : m_controllerList)
		{
			const bool isSelected = id == controller;
			if (ImGui::Selectable(str.data(), &isSelected))
			{
				controller = id;
			}
			if (isSelected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
}

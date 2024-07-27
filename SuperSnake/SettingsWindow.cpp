#include "SettingsWindow.hpp"
#include <imgui.h>
#include <imgui_stdlib.h>

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
		auto controller = GameController::FromGamepadInfo(gamepad);
		m_controllerList.emplace(
			controller,
			fmt::format("Gamepad: {}##{}", gamepad.name.toUTF8(), controller.gamepadUid)
		);
	}

	ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	ImGui::Begin("Settings", nullptr,
		ImGuiWindowFlags_::ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_::ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysAutoResize);
	{
		ImGui::SeparatorText("Preset");

		auto presets = GetGamePresets();
		bool presetsModified = false;
		if (presets.empty())
		{
			ImGui::TextDisabled("No presets");
		}
		else
		{
			for (auto itr = presets.begin(); itr != presets.end(); itr++)
			{
				auto& [presettingsName, presettings] = *itr;
				if (ImGui::Button(Unicode::ToUTF8(presettingsName).data()))
				{
					m_presetName = Unicode::ToUTF8(presettingsName);
					m_settings = presettings;

					auto gamepads = System::EnumerateGamepads();
					for (auto& controller : m_settings.selectedControllers)
					{
						if (controller.kind == GameController::Kind::Gamepad)
						{
							auto itr = std::find_if(gamepads.cbegin(), gamepads.cend(), [&](const GamepadInfo& info) {
								return GameController::FromGamepadInfo(info).gamepadUid == controller.gamepadUid;
							});

							if (itr == gamepads.end())
							{
								controller = GameController::Unselected();
							}
							else
							{
								controller.index = itr->playerIndex;
							}
						}
						else if (controller.kind == GameController::Kind::Solver)
						{
							if (controller.index >= Solvers.size())
							{
								controller = GameController::Unselected();
							}
						}
					}
				}
				else if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
				{
					presets.erase(itr);
					presetsModified = true;
				}

				ImGui::SameLine();
			}
			ImGui::Dummy({ 0, 0 });
		}

		{
			bool returnPressed = ImGui::InputTextWithHint("##SaveAsPreset", "Preset Name", &m_presetName, ImGuiInputTextFlags_EnterReturnsTrue);
			bool isPresetNameEmpty = m_presetName.empty();

			ImGui::SameLine();
			ImGui::BeginDisabled(isPresetNameEmpty);
			if ((ImGui::Button("Save Preset") || returnPressed) && !isPresetNameEmpty)
			{
				presets[Unicode::FromUTF8(m_presetName)] = m_settings;
				presetsModified = true;
			}
			ImGui::EndDisabled();
		}

		if (presetsModified)
		{
			SetGamePresets(presets);
			SaveConfig();
		}

		ImGui::SeparatorText("Config");

		ImGui::BulletText("Field");
		ImGui::Indent();
		{
			if (ImGui::InputInt("width", &m_settings.fieldSize.x))
			{
				m_settings.fieldSize.x = Max(m_settings.fieldSize.x, 2);
			}
			if (ImGui::InputInt("height", &m_settings.fieldSize.y))
			{
				m_settings.fieldSize.y = Max(m_settings.fieldSize.y, 2);
			}
		}
		ImGui::Unindent();

		ImGui::BulletText("Snake");
		ImGui::Indent();
		{
			int count = m_settings.selectedControllers.size();
			if (ImGui::InputInt("count", &count))
			{
				count = Clamp(count, 1, 4);
				m_settings.selectedControllers.resize(count, GameController{ .kind = GameController::Kind::Unselected });
			}
		}
		ImGui::Unindent();

		ImGui::BulletText("Controllers");
		ImGui::Indent();
		{
			for (int i : Iota(m_settings.selectedControllers.size()))
			{
				auto& controller = m_settings.selectedControllers[i];
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

		ImGui::BulletText("Options");
		ImGui::Indent();
		{
			ImGui::Checkbox("Hide Confirmed Action", &m_settings.hideConfirmedAction);
		}
		ImGui::Unindent();

		ImGui::Separator();

		ImGui::BeginDisabled(m_settings.selectedControllers.includes_if([](const GameController c) {
			return c.kind == GameController::Kind::Unselected;
			}));
		{
			if (ImGui::Button("Start!", { 70, 30 }))
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

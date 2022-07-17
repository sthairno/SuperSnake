#include "KeyConfigWindow.hpp"
#include <imgui.h>

template<int idx>
static bool BuildKeyConfigUI(const std::string& idName, uint8& id, Optional<uint8> pressedId, int& selectedIdx)
{
	bool isSelected = idx == selectedIdx;
	const std::string label = isSelected
		? fmt::format("[...]##{}", idx)
		: fmt::format("{}##{}", idName, idx);

	ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(.5f, .5f));
	if (ImGui::Selectable(label.data(), &isSelected, ImGuiSelectableFlags_None, {140, 18}))
	{
		if (isSelected)
		{
			selectedIdx = idx;
		}
		else
		{
			selectedIdx = -1;
		}
	}
	ImGui::PopStyleVar();

	if (isSelected && pressedId)
	{
		selectedIdx = -1;
		if (id != *pressedId)
		{
			id = *pressedId;
			return true;
		}
	}
	return false;
}

static bool IsButtonPressed(const detail::Gamepad_impl& gamepad, uint8 id)
{
	return id < gamepad.buttons.size() && gamepad.buttons[id].pressed();
}

static double GetAxisValue(const detail::Gamepad_impl& gamepad, uint8 id)
{
	const uint8 idx = id / 2;
	if (idx >= gamepad.axes.size())
	{
		return 0.0;
	}
	else
	{
		const double mul = id % 2 ? 1.0 : -1.0;
		return gamepad.axes[idx] * mul;
	}
}

static void BuildButtonText(const char* text, bool pressed)
{
	if (pressed)
	{
		ImGui::TextColored({ 1.f, 1.f, 0.f, 1.f }, text);
	}
	else
	{
		ImGui::Text(text);
	}
}

static void BuildStickPreview(float x, float y)
{
	static const ImVec2 rectSize{ 60, 60 };
	static const float stickR = 6;

	const ImGuiStyle& style = ImGui::GetStyle();
	ImDrawList* drawList = ImGui::GetWindowDrawList();

	ImVec2 rectTL = ImGui::GetCursorScreenPos();
	rectTL.x += stickR;
	rectTL.y += stickR;
	ImVec2 rectBR{ rectTL.x + rectSize.x, rectTL.y + rectSize.y };
	ImVec2 rectCenter{ rectTL.x + rectSize.x / 2, rectTL.y + rectSize.y / 2 };

	drawList->AddRectFilled(rectTL, rectBR, ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_PopupBg]));
	drawList->AddRect(rectTL, rectBR, ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_Separator]));

	drawList->AddCircleFilled({ rectCenter.x + x * rectSize.x / 2, rectCenter.y + y * rectSize.y / 2 }, stickR, ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_PlotHistogram]));

	ImGui::Dummy({ rectSize.x + stickR * 2, rectSize.y + stickR * 2 });
}

void KeyConfigWindow::renderWindow(bool& visible)
{
	ImGui::Begin(std::format("KeyConfig: {}##{}", target.name.toUTF8(), target.vendorID ^ target.productID).data(), &visible, ImGuiWindowFlags_AlwaysAutoResize);
	if (visible)
	{
		if (m_setFocus)
		{
			ImGui::SetWindowFocus();
		}

		auto config = GetKeyConfig(target);
		bool valueChanged = false;
		auto& gamepad = Gamepad(target.playerIndex);
		const bool isConnected =
			gamepad.getInfo().vendorID == target.vendorID &&
			gamepad.getInfo().productID == target.productID &&
			gamepad.isConnected();
		Optional<uint8> pressedButtonId;
		Optional<uint8> inputtedStickId;

		if (isConnected)
		{
			for (auto& button : gamepad.buttons)
			{
				if (button.down())
				{
					pressedButtonId = button.code();
					break;
				}
			}
			for (auto [idx, axis] : Indexed(gamepad.axes))
			{
				if (axis < -0.5)
				{
					inputtedStickId = idx + 1;
					break;
				}
				if (axis > 0.5)
				{
					inputtedStickId = idx;
				}
			}
		}
		else
		{
			m_selectedIdx = -1;
		}

		ImGui::BeginDisabled(not isConnected);

		valueChanged |= BuildKeyConfigUI<0>(fmt::format("Button {}", config.selectButtonId), config.selectButtonId, pressedButtonId, m_selectedIdx);
		ImGui::SameLine();
		BuildButtonText("Select", IsButtonPressed(gamepad, config.selectButtonId));

		valueChanged |= BuildKeyConfigUI<1>(fmt::format("Button {}", config.cancelButtonId), config.cancelButtonId, pressedButtonId, m_selectedIdx);
		ImGui::SameLine();
		BuildButtonText("Cancel", IsButtonPressed(gamepad, config.cancelButtonId));

		valueChanged |= BuildKeyConfigUI<2>(fmt::format("Button {}", config.leftButtonId), config.leftButtonId, pressedButtonId, m_selectedIdx);
		ImGui::SameLine();
		BuildButtonText("L Trigger", IsButtonPressed(gamepad, config.leftButtonId));

		valueChanged |= BuildKeyConfigUI<3>(fmt::format("Button {}", config.rightButtonId), config.rightButtonId, pressedButtonId, m_selectedIdx);
		ImGui::SameLine();
		BuildButtonText("R Trigger", IsButtonPressed(gamepad, config.rightButtonId));

		valueChanged |= BuildKeyConfigUI<4>(fmt::format("Axis {}{}", config.stickXaxisId / 2, config.stickXaxisId % 2 ? '-' : '+'), config.stickXaxisId, inputtedStickId, m_selectedIdx);
		ImGui::SameLine();
		ImGui::Text("Stick Right");

		valueChanged |= BuildKeyConfigUI<5>(fmt::format("Axis {}{}", config.stickYaxisId / 2, config.stickYaxisId % 2 ? '-' : '+'), config.stickYaxisId, inputtedStickId, m_selectedIdx);
		ImGui::SameLine();
		ImGui::Text("Stick Up");

		BuildStickPreview(GetAxisValue(gamepad, config.stickXaxisId), -GetAxisValue(gamepad, config.stickYaxisId));

		ImGui::EndDisabled();
		
		if (valueChanged)
		{
			SetKeyConfig(target, config);
			SaveConfig();
		}
	}
	else
	{
		m_selectedIdx = -1;
	}
	ImGui::End();

	m_setFocus = false;
}

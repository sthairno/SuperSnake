#include <Siv3D.hpp>
#include <imgui.h>
#include "imgui_impl_s3d.h"
#include "DearImGuiAddon.hpp"

/// @brief アドオンの登録時の初期化処理を記述します。
/// @remark この関数が false を返すとアドオンの登録は失敗します。
/// @return アドオンの初期化に成功した場合 true, それ以外の場合は false

bool DearImGuiAddon::init()
{
	ImGui::CreateContext();
	ImGui_Impls3d_Init();

	return true;
}

/// @brief アドオンの毎フレームの更新処理を記述します。
/// @remark この関数が false を返すと `System::Update()` は false を返します。
/// @return アドオンの更新に成功した場合 true, それ以外の場合は false

bool DearImGuiAddon::update()
{
	ImGui_Impls3d_NewFrame();
	ImGui::NewFrame();

	m_firstFrame = false;
	return true;
}

/// @brief アドオンの毎フレームの描画処理を記述します。

void DearImGuiAddon::draw() const
{
	if (m_firstFrame)
	{
		return;
	}

	ImGui::Render();
	ImGui_Impls3d_RenderDrawData(::ImGui::GetDrawData());
}

DearImGuiAddon::~DearImGuiAddon()
{
	ImGui_Impls3d_Shutdown();
}

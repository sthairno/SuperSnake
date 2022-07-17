#pragma once
struct imgui_impl_s3d;

ImTextureID GetImTextureID(std::shared_ptr<Texture> texture);
IMGUI_IMPL_API bool     ImGui_Impls3d_Init();
IMGUI_IMPL_API void     ImGui_Impls3d_Shutdown();
IMGUI_IMPL_API void     ImGui_Impls3d_NewFrame();
IMGUI_IMPL_API void     ImGui_Impls3d_RenderDrawData(ImDrawData* draw_data);
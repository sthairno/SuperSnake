#include <imgui.h>
#include <Siv3D.hpp>
#include "imgui_impl_s3d.h"

// Data

struct ImGuiToS3DKey
{
	ImGuiKey imgui;
	Input s3d;
	bool s3dCtrl = false;
};

static std::shared_ptr<Texture> g_Texture;
static std::unique_ptr<std::string> g_ClipboardTextData = NULL;

static Point ImeDrawPos(0, 0);
static std::unique_ptr<Font> ImeInputFont;
static String ImeEditingText;
static bool ImeLockKey = false;
static bool ImeLockEnter = false;

const Array<ImGuiToS3DKey> ImGuiToS3DKeyList = {
	ImGuiToS3DKey{ImGuiKey_Tab,KeyTab},
	ImGuiToS3DKey{ImGuiKey_LeftArrow,KeyLeft},
	ImGuiToS3DKey{ImGuiKey_RightArrow,KeyRight},
	ImGuiToS3DKey{ImGuiKey_UpArrow,KeyUp},
	ImGuiToS3DKey{ImGuiKey_DownArrow,KeyDown},
	ImGuiToS3DKey{ImGuiKey_PageUp,KeyPageUp},
	ImGuiToS3DKey{ImGuiKey_PageDown,KeyPageDown},
	ImGuiToS3DKey{ImGuiKey_Home,KeyHome},
	ImGuiToS3DKey{ImGuiKey_End,KeyEnd},
	ImGuiToS3DKey{ImGuiKey_Insert,KeyInsert},
	ImGuiToS3DKey{ImGuiKey_Delete,KeyDelete},
	ImGuiToS3DKey{ImGuiKey_Backspace,KeyBackspace},
	ImGuiToS3DKey{ImGuiKey_Space,KeySpace},
	ImGuiToS3DKey{ImGuiKey_Enter,KeyEnter},
	ImGuiToS3DKey{ImGuiKey_Escape,KeyEscape},
	ImGuiToS3DKey{ImGuiKey_KeyPadEnter,KeyNumEnter},
	ImGuiToS3DKey{ImGuiKey_A,KeyA,true},
	ImGuiToS3DKey{ImGuiKey_C,KeyC,true},
	ImGuiToS3DKey{ImGuiKey_V,KeyV,true},
	ImGuiToS3DKey{ImGuiKey_X,KeyX,true},
	ImGuiToS3DKey{ImGuiKey_Y,KeyY,true},
	ImGuiToS3DKey{ImGuiKey_Z,KeyZ,true}
};

static ImTextureID GetImTextureID(std::shared_ptr<Texture> texture)
{
	return (ImTextureID)(new std::shared_ptr<Texture>(texture));
}

static const char* ImGui_Impls3d_GetClipboardText(void*)
{
	if (g_ClipboardTextData)
		g_ClipboardTextData.reset();
	String str;
	Clipboard::GetText(str);
	g_ClipboardTextData = std::make_unique<std::string>(str.narrow());
	return g_ClipboardTextData->c_str();
}

static void ImGui_Impls3d_SetClipboardText(void*, const char* text)
{
	Clipboard::SetText(s3d::Unicode::Widen(std::string(text)));
}

inline static ImVec2 GetImVec2(Vec2 vec)
{
	return ImVec2(vec.x, vec.y);
}

inline static Vec2 GetSivVec2(ImVec2 vec)
{
	return Vec2(vec.x, vec.y);
}

bool ImGui_Impls3d_Init()
{
	ImGuiIO& io = ImGui::GetIO();

	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

	io.BackendRendererName = "imgui_impl_s3d";
	io.BackendPlatformName = "imgui_impl_s3d";

	io.SetClipboardTextFn = ImGui_Impls3d_SetClipboardText;
	io.GetClipboardTextFn = ImGui_Impls3d_GetClipboardText;

	if (ImeInputFont)
		ImeInputFont.reset();
	ImeInputFont = std::make_unique<Font>(15);
	io.ImeSetInputScreenPosFn = [](int x, int y) { ImeDrawPos = Point(x, y); };

	for (auto i2s : ImGuiToS3DKeyList)
	{
		if (i2s.s3dCtrl)
		{
			io.KeyMap[i2s.imgui] = KeyControl.code() | i2s.s3d.code();
		}
		else
		{
			io.KeyMap[i2s.imgui] = i2s.s3d.code();
		}
	}

	io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
	return true;
}

void ImGui_Impls3d_Shutdown()
{
	g_Texture.~shared_ptr();
}

bool ImGui_Impls3d_CreateDeviceObjects()
{
	// Build texture atlas
	ImGuiIO& io = ImGui::GetIO();
	unsigned char* pixels;
	int width, height;
	io.Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);

	// Create texture
	g_Texture = std::make_shared<Texture>(Image::Generate({ width, height }, [&]()
		{
			return Color(255, 255, 255, (unsigned int)*(pixels++));
		}));

	// Store our identifier
	io.Fonts->TexID = GetImTextureID(g_Texture);

	return true;
}

void ImGui_Impls3d_NewFrame()
{
	if (!g_Texture)
		ImGui_Impls3d_CreateDeviceObjects();
	ImGuiIO& io = ImGui::GetIO();

	//Display
	{
		io.DisplaySize = GetImVec2(Scene::Size());
	}
	//Keyboard
	{
		ImeLockKey = false;
		if (io.WantTextInput)
		{
			//IMEを使う
			int lastLen = ImeEditingText.length();
			ImeEditingText = TextInput::GetEditingText();
			if (ImeEditingText.length() == 0)
			{
				auto input = TextInput::GetRawInput();
				if (input.length() > 0)
				{
					io.AddInputCharactersUTF8(input.toUTF8().c_str());
				}
			}
			else
			{
				ImeLockKey = true;
				ImeLockEnter = true;
			}
		}
		else
		{
			//IMEを使わない
			if (ImeEditingText.length() > 0)
			{
				ImeEditingText.clear();
			}
		}

		io.KeyShift = KeyLShift.pressed() || KeyRShift.pressed();
		io.KeyCtrl = KeyControl.pressed();
		io.KeyAlt = KeyAlt.pressed();
		if (!ImeLockKey)
		{
			if (ImeLockEnter && !KeyEnter.pressed())
			{
				ImeLockEnter = false;
			}
			for (auto i2s : ImGuiToS3DKeyList)
			{
				if (i2s.imgui == ImGuiKey_Enter && ImeLockEnter)
				{
					io.KeysDown[io.KeyMap[i2s.imgui]] = false;
					continue;
				}
				if (i2s.s3dCtrl)
				{
					io.KeysDown[io.KeyMap[i2s.imgui]] = (KeyControl | i2s.s3d).pressed();
				}
				else
				{
					io.KeysDown[io.KeyMap[i2s.imgui]] = i2s.s3d.pressed();
				}
			}
		}
	}
	//Cursor
	{
		if (!(io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange))
		{
			if (io.MouseDrawCursor)
			{
				Cursor::RequestStyle(CursorStyle::Hidden);
			}
			else
			{
				CursorStyle siv_cursor = CursorStyle::Default;
				switch (ImGui::GetMouseCursor())
				{
				case ImGuiMouseCursor_None:			siv_cursor = CursorStyle::Hidden; break;
				case ImGuiMouseCursor_Arrow:        siv_cursor = CursorStyle::Arrow; break;
				case ImGuiMouseCursor_TextInput:    siv_cursor = CursorStyle::IBeam; break;
				case ImGuiMouseCursor_ResizeAll:    siv_cursor = CursorStyle::ResizeAll; break;
				case ImGuiMouseCursor_ResizeEW:     siv_cursor = CursorStyle::ResizeLeftRight; break;
				case ImGuiMouseCursor_ResizeNS:     siv_cursor = CursorStyle::ResizeUpDown; break;
				case ImGuiMouseCursor_ResizeNESW:   siv_cursor = CursorStyle::ResizeNESW; break;
				case ImGuiMouseCursor_ResizeNWSE:   siv_cursor = CursorStyle::ResizeNWSE; break;
				case ImGuiMouseCursor_Hand:         siv_cursor = CursorStyle::Hand; break;
				}
				Cursor::RequestStyle(siv_cursor);
			}
		}
	}
	//Mouse
	{
		if (io.WantSetMousePos)
		{
			Cursor::SetPos(io.MousePos.x, io.MousePos.y);
		}

		io.MousePos = GetImVec2(Cursor::PosF());

		io.MouseWheel -= Mouse::Wheel();
		io.MouseWheelH -= Mouse::WheelH();

		io.MouseDown[ImGuiMouseButton_Left] = MouseL.pressed();
		io.MouseDown[ImGuiMouseButton_Right] = MouseR.pressed();
		io.MouseDown[ImGuiMouseButton_Middle] = MouseM.pressed();
	}
}

void ImGui_Impls3d_RenderDrawData(ImDrawData* draw_data)
{
	if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f)
		return;
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		static Buffer2D sp;

		sp.vertices = Array<Vertex2D>(cmd_list->VtxBuffer.Size);
		for (int i = 0; i < cmd_list->VtxBuffer.Size; i++)
		{
			const ImDrawVert& src_v = cmd_list->VtxBuffer[i];
			Vertex2D& dst_v = sp.vertices[i];
			dst_v.pos.x = src_v.pos.x;
			dst_v.pos.y = src_v.pos.y;
			dst_v.tex.x = src_v.uv.x;
			dst_v.tex.y = src_v.uv.y;
			unsigned char* c = (unsigned char*)&src_v.col;
			dst_v.color = ColorF(((unsigned int)c[0]) / 255.0f, ((unsigned int)c[1]) / 255.0f, ((unsigned int)c[2]) / 255.0f, ((unsigned int)c[3]) / 255.0f).toFloat4();
		}
		assert(!(cmd_list->IdxBuffer.Size % 3));
		sp.indices.resize(cmd_list->IdxBuffer.Size / 3);
		assert(sp.indices.size_bytes() == cmd_list->IdxBuffer.size_in_bytes());
		memcpy(sp.indices.data(), cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.size_in_bytes());

		{
			//クリッピング領域を有効にする
			RasterizerState rasterizer = RasterizerState::Default2D;
			rasterizer.scissorEnable = true;
			ScopedRenderStates2D r(rasterizer);

			uint32 indexOffset = 0;
			ImVec2 clip_off = draw_data->DisplayPos;
			for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
			{
				const ImDrawCmd& pcmd = cmd_list->CmdBuffer[cmd_i];
				const uint32 triangleCnt = pcmd.ElemCount / 3;
				if (pcmd.UserCallback)
				{
					if (pcmd.UserCallback == ImDrawCallback_ResetRenderState)
					{
						Graphics2D::SetScissorRect(Scene::Rect());
					}
					else
					{
						pcmd.UserCallback(cmd_list, &pcmd);
					}
				}
				else
				{
					// Draw
					std::shared_ptr<Texture> texture = *((std::shared_ptr<Texture>*)pcmd.TextureId);
					Graphics2D::SetScissorRect(Rect(pcmd.ClipRect.x - clip_off.x, pcmd.ClipRect.y - clip_off.y, pcmd.ClipRect.z - pcmd.ClipRect.x, pcmd.ClipRect.w - pcmd.ClipRect.y));
					sp.drawSubset(indexOffset, triangleCnt, *texture);
				}
				indexOffset += triangleCnt;
			}
		}
	}
	if (ImeEditingText.length() > 0)
	{
		(*ImeInputFont)(ImeEditingText).draw(ImeDrawPos);
	}
}

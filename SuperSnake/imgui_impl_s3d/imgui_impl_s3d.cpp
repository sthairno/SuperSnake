#include <imgui.h> // v1.88
#include <Siv3D.hpp> // OpenSiv3D v0.6.5
#include "imgui_impl_s3d.h"

struct ImeWindowContext
{
	Vec2 pos;

	double lineHeight;
};

struct ImGuiImpls3dContext
{
	std::string clipboardData;

	uint64 keyDownMinTime = 0;

	std::array<uint64, 512> keyDownTimeList;

	Texture fontTexture;

	Optional<ImeWindowContext> imeWindow;

	std::unordered_map < ImTextureID, Texture, decltype([](ImTextureID id) { return reinterpret_cast<size_t>(id); }) > textureDic;
};

const static std::unordered_map<uint8, ImGuiKey> KeyId2ImGuiKeyDic{
	{KeyTab.code(), ImGuiKey_Tab},
	{KeyLeft.code(), ImGuiKey_LeftArrow},
	{KeyRight.code(), ImGuiKey_RightArrow},
	{KeyUp.code(), ImGuiKey_UpArrow},
	{KeyDown.code(), ImGuiKey_DownArrow},
	{KeyPageUp.code(), ImGuiKey_PageUp},
	{KeyPageDown.code(), ImGuiKey_PageDown},
	{KeyHome.code(), ImGuiKey_Home},
	{KeyEnd.code(), ImGuiKey_End},
	{KeyInsert.code(), ImGuiKey_Insert},
	{KeyDelete.code(), ImGuiKey_Delete},
	{KeyBackspace.code(), ImGuiKey_Backspace},
	{KeySpace.code(), ImGuiKey_Space},
	{KeyEnter.code(), ImGuiKey_Enter},
	{KeyEscape.code(), ImGuiKey_Escape},
	{KeyLControl.code(), ImGuiKey_LeftCtrl},
	{KeyLShift.code(), ImGuiKey_LeftShift},
	{KeyLAlt.code(), ImGuiKey_LeftAlt},
	{KeyRControl.code(), ImGuiKey_RightCtrl},
	{KeyRShift.code(), ImGuiKey_RightShift},
	{KeyRAlt.code(), ImGuiKey_RightAlt},
	{Key0.code(), ImGuiKey_0},
	{Key1.code(), ImGuiKey_1},
	{Key2.code(), ImGuiKey_2},
	{Key3.code(), ImGuiKey_3},
	{Key4.code(), ImGuiKey_4},
	{Key5.code(), ImGuiKey_5},
	{Key6.code(), ImGuiKey_6},
	{Key7.code(), ImGuiKey_7},
	{Key8.code(), ImGuiKey_8},
	{Key9.code(), ImGuiKey_9},
	{KeyA.code(), ImGuiKey_A},
	{KeyB.code(), ImGuiKey_B},
	{KeyC.code(), ImGuiKey_C},
	{KeyD.code(), ImGuiKey_D},
	{KeyE.code(), ImGuiKey_E},
	{KeyF.code(), ImGuiKey_F},
	{KeyG.code(), ImGuiKey_G},
	{KeyH.code(), ImGuiKey_H},
	{KeyI.code(), ImGuiKey_I},
	{KeyJ.code(), ImGuiKey_J},
	{KeyK.code(), ImGuiKey_K},
	{KeyL.code(), ImGuiKey_L},
	{KeyM.code(), ImGuiKey_M},
	{KeyN.code(), ImGuiKey_N},
	{KeyO.code(), ImGuiKey_O},
	{KeyP.code(), ImGuiKey_P},
	{KeyQ.code(), ImGuiKey_Q},
	{KeyR.code(), ImGuiKey_R},
	{KeyS.code(), ImGuiKey_S},
	{KeyT.code(), ImGuiKey_T},
	{KeyU.code(), ImGuiKey_U},
	{KeyV.code(), ImGuiKey_V},
	{KeyW.code(), ImGuiKey_W},
	{KeyX.code(), ImGuiKey_X},
	{KeyY.code(), ImGuiKey_Y},
	{KeyZ.code(), ImGuiKey_Z},
	{KeyF1.code(), ImGuiKey_F1},
	{KeyF2.code(), ImGuiKey_F2},
	{KeyF3.code(), ImGuiKey_F3},
	{KeyF4.code(), ImGuiKey_F4},
	{KeyF5.code(), ImGuiKey_F5},
	{KeyF6.code(), ImGuiKey_F6},
	{KeyF7.code(), ImGuiKey_F7},
	{KeyF8.code(), ImGuiKey_F8},
	{KeyF9.code(), ImGuiKey_F9},
	{KeyF10.code(), ImGuiKey_F10},
	{KeyF11.code(), ImGuiKey_F11},
	{KeyF12.code(), ImGuiKey_F12},
	{KeyApostrophe_US.code(), ImGuiKey_Apostrophe},
	{KeyComma.code(), ImGuiKey_Comma},
	{KeyMinus.code(), ImGuiKey_Minus},
	{KeyPeriod.code(), ImGuiKey_Period},
	{KeySlash.code(), ImGuiKey_Slash},
	{KeySemicolon_US.code(), ImGuiKey_Semicolon},
	{KeyEqual_US.code(), ImGuiKey_Equal},
	{KeyLBracket.code(), ImGuiKey_LeftBracket},
	{KeyBackslash_US.code(), ImGuiKey_Backslash},
	{KeyRBracket.code(), ImGuiKey_RightBracket},
	{KeyGraveAccent.code(), ImGuiKey_GraveAccent},
	{KeyNumLock.code(), ImGuiKey_NumLock},
	{KeyPrintScreen.code(), ImGuiKey_PrintScreen},
	{KeyPause.code(), ImGuiKey_Pause},
	{KeyNum0.code(), ImGuiKey_Keypad0},
	{KeyNum1.code(), ImGuiKey_Keypad1},
	{KeyNum2.code(), ImGuiKey_Keypad2},
	{KeyNum3.code(), ImGuiKey_Keypad3},
	{KeyNum4.code(), ImGuiKey_Keypad4},
	{KeyNum5.code(), ImGuiKey_Keypad5},
	{KeyNum6.code(), ImGuiKey_Keypad6},
	{KeyNum7.code(), ImGuiKey_Keypad7},
	{KeyNum8.code(), ImGuiKey_Keypad8},
	{KeyNum9.code(), ImGuiKey_Keypad9},
	{KeyNumDecimal.code(), ImGuiKey_KeypadDecimal},
	{KeyNumDivide.code(), ImGuiKey_KeypadDivide},
	{KeyNumMultiply.code(), ImGuiKey_KeypadMultiply},
	{KeyNumSubtract.code(), ImGuiKey_KeypadSubtract},
	{KeyNumAdd.code(), ImGuiKey_KeypadAdd},
	{KeyNumEnter.code(), ImGuiKey_KeypadEnter},
	{KeyControl.code(), ImGuiKey_ModCtrl},
	{KeyShift.code(), ImGuiKey_ModShift},
	{KeyAlt.code(), ImGuiKey_ModAlt},
};

inline ImVec2 ToImVec2(Point src)
{
	return ImVec2{ static_cast<float>(src.x), static_cast<float>(src.y) };
}

inline ImVec2 ToImVec2(Float2 src)
{
	return ImVec2{ src.x, src.y };
}

inline ImVec2 ToImVec2(Vec2 src)
{
	return ImVec2{ static_cast<float>(src.x), static_cast<float>(src.y) };
}

inline Float2 ToFloat2(ImVec2 src)
{
	return Float2{ src.x, src.y };
}

static ImGuiKey ToImGuiKey(Input input)
{
	auto itr = KeyId2ImGuiKeyDic.find(input.code());
	return itr == KeyId2ImGuiKeyDic.cend()
		? ImGuiKey_None
		: itr->second;
}

static std::unique_ptr<ImGuiImpls3dContext> Context;

static const char* GetClipboardTextCallback(void*)
{
	Context->clipboardData.clear();

	String buff;
	if (not Clipboard::GetText(buff))
	{
		return NULL;
	}

	Context->clipboardData = Unicode::ToUTF8(buff);
	return Context->clipboardData.c_str();
}

static void SetClipboardTextCallback(void*, const char* text)
{
	Clipboard::SetText(Unicode::FromUTF8(text));
}

static void SetImeDataCallback(ImGuiViewport* viewport, ImGuiPlatformImeData* data)
{
	if (data->WantVisible)
	{
		Context->imeWindow = ImeWindowContext{
			.pos = ToFloat2(data->InputPos),
			.lineHeight = data->InputLineHeight
		};
	}
	else
	{
		Context->imeWindow.reset();
	}
}

static void RenderImeWindow()
{
	if (not Context->imeWindow)
	{
		return;
	}

	ImeWindowContext& imeWindow = Context->imeWindow.value();

	const auto& font = SimpleGUI::GetFont();
	const auto drawableText = font(TextInput::GetEditingText());
	const double size = imeWindow.lineHeight * font.fontSize() / (font.fontSize() + font.descender());

	drawableText.region(size, imeWindow.pos)
		.draw(Palette::White);
	drawableText
		.draw(size, imeWindow.pos, Palette::Black);
}

static void CreateFontsTexture()
{
	ImGuiIO& io = ImGui::GetIO();

	unsigned char* pixels;
	Size size;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &size.x, &size.y);

	Image image(size);
	memcpy_s(image.dataAsUint8(), image.size_bytes(), pixels, size.x * size.y * 4);

	Texture texture(image);
	Context->fontTexture = texture;

	ImTextureID id = ImGui_Impls3d_RegisterTexture(texture);
	io.Fonts->SetTexID(id);
}

///// API /////

ImTextureID ImGui_Impls3d_RegisterTexture(Texture& tex)
{
	ImTextureID id = reinterpret_cast<void*>(tex.id().value());
	Context->textureDic.try_emplace(id, tex);
	return id;
}

void ImGui_Impls3d_UnregisterTexture(Texture& tex)
{
	ImTextureID id = reinterpret_cast<void*>(tex.id().value());
	Context->textureDic.erase(id);
}

Texture ImGui_Impls3d_GetTexture(ImTextureID id)
{
	return Context->textureDic.at(id);
}

bool ImGui_Impls3d_Init()
{
	Context = std::make_unique<ImGuiImpls3dContext>();
	Context->keyDownTimeList.fill(0);

	ImGuiIO& io = ImGui::GetIO();

	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
	// io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

	io.BackendPlatformName = "imgui_impl_s3d";
	io.BackendRendererName = "imgui_impl_s3d";

	io.GetClipboardTextFn = &GetClipboardTextCallback;
	io.SetClipboardTextFn = &SetClipboardTextCallback;
	io.SetPlatformImeDataFn = &SetImeDataCallback;

	return true;
}

void ImGui_Impls3d_Shutdown()
{
	Context.reset();
}

void ImGui_Impls3d_NewFrame()
{
	if (not Context->fontTexture)
	{
		CreateFontsTexture();
	}

	ImGuiIO& io = ImGui::GetIO();
	uint64 currentTime = Time::GetMillisec();

	//Display
	{
		io.DisplaySize = ToImVec2(Scene::Size());
		io.DeltaTime = Scene::DeltaTime();
	}

	// TextInput
	{
		String editingText = TextInput::GetEditingText();
		String input = TextInput::GetRawInput();
		if (editingText)
		{
			// 文字変換との競合を防止するため、変換中はキーボード入力を一時的に無効化する
			Context->keyDownMinTime = currentTime + 100;
		}
		if (input)
		{
			io.AddInputCharactersUTF8(Unicode::ToUTF8(input).c_str());
		}
	}

	//Keyboard
	{
		for (auto key : Keyboard::GetAllInputs())
		{
			uint64& keyDownTime = Context->keyDownTimeList[key.code()];
			if (key.down())
			{
				keyDownTime = currentTime;
			}

			const bool pressed = key.pressed() && keyDownTime >= Context->keyDownMinTime;
			io.AddKeyEvent(ToImGuiKey(key), pressed);
		}
	}

	//Cursor
	{
		if (io.WantSetMousePos)
		{
			Cursor::SetPos(io.MousePos.x, io.MousePos.y);
		}

		if (not (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange))
		{
			if (io.MouseDrawCursor)
			{
				Cursor::RequestStyle(CursorStyle::Hidden);
			}
			else
			{
				CursorStyle s3dStyle;
				switch (ImGui::GetMouseCursor())
				{
				case ImGuiMouseCursor_None: s3dStyle = CursorStyle::Hidden; break;
				case ImGuiMouseCursor_Arrow: s3dStyle = CursorStyle::Arrow; break;
				case ImGuiMouseCursor_TextInput: s3dStyle = CursorStyle::IBeam; break;
				case ImGuiMouseCursor_ResizeAll: s3dStyle = CursorStyle::ResizeAll; break;
				case ImGuiMouseCursor_ResizeEW: s3dStyle = CursorStyle::ResizeLeftRight; break;
				case ImGuiMouseCursor_ResizeNS: s3dStyle = CursorStyle::ResizeUpDown; break;
				case ImGuiMouseCursor_ResizeNESW: s3dStyle = CursorStyle::ResizeNESW; break;
				case ImGuiMouseCursor_ResizeNWSE: s3dStyle = CursorStyle::ResizeNWSE; break;
				case ImGuiMouseCursor_Hand: s3dStyle = CursorStyle::Hand; break;
				case ImGuiMouseCursor_NotAllowed: s3dStyle = CursorStyle::NotAllowed; break;
				default: s3dStyle = CursorStyle::Default; break;
				}
				Cursor::RequestStyle(s3dStyle);
			}
		}
	}

	//Mouse
	{
		const Vec2 mousePos = Cursor::PosF();
		const Vec2 wheel{ Mouse::WheelH(), Mouse::Wheel() };

		io.AddMousePosEvent(mousePos.x, mousePos.y);
		io.AddMouseWheelEvent(-wheel.x, -wheel.y);
		for (const Input& input : Mouse::GetAllInputs())
		{
			io.AddMouseButtonEvent(input.code(), input.pressed());
		}

		if (io.WantCaptureMouse)
		{
			Mouse::ClearLRInput();
		}
	}

	// Window
	{
		const WindowState& state = Window::GetState();
		io.AddFocusEvent(!state.sizeMove && state.focused);
	}
}

void ImGui_Impls3d_RenderDrawData(ImDrawData* draw_data)
{
	RasterizerState rasterizer = RasterizerState::Default2D;
	rasterizer.scissorEnable = true;
	Rect prevScissorRect = Graphics2D::GetScissorRect();

	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		static Buffer2D sp;

		sp.vertices.resize(cmd_list->VtxBuffer.Size);
		for (int i = 0; i < cmd_list->VtxBuffer.Size; i++)
		{
			const ImDrawVert& srcVtx = cmd_list->VtxBuffer[i];
			Vertex2D& dstVtx = sp.vertices[i];

			dstVtx.pos.x = srcVtx.pos.x;
			dstVtx.pos.y = srcVtx.pos.y;
			dstVtx.tex.x = srcVtx.uv.x;
			dstVtx.tex.y = srcVtx.uv.y;

			const uint8* c = (uint8*)(&srcVtx.col);
			dstVtx.color = ColorF(Color(c[0], c[1], c[2], c[3])).toFloat4();
		}
		sp.indices.resize(cmd_list->IdxBuffer.Size / 3);
		memcpy(sp.indices.data(), cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.size_in_bytes());

		ScopedRenderStates2D r(rasterizer);

		// uint32 vtxOffset = 0;
		uint32 idxOffset = 0;
		ImVec2 clipOffset = draw_data->DisplayPos;

		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
		{
			const ImDrawCmd& pcmd = cmd_list->CmdBuffer[cmd_i];
			const uint32 triangleCnt = pcmd.ElemCount / 3;
			if (pcmd.UserCallback)
			{
				if (pcmd.UserCallback == ImDrawCallback_ResetRenderState)
				{
					Graphics2D::SetScissorRect(prevScissorRect);
				}
				else
				{
					pcmd.UserCallback(cmd_list, &pcmd);
				}
			}
			else
			{
				Texture texture = ImGui_Impls3d_GetTexture(pcmd.TextureId);
				Graphics2D::SetScissorRect(Rect(pcmd.ClipRect.x - clipOffset.x, pcmd.ClipRect.y - clipOffset.y, pcmd.ClipRect.z - pcmd.ClipRect.x, pcmd.ClipRect.w - pcmd.ClipRect.y));
				sp.drawSubset(idxOffset, triangleCnt, texture);

			}
			idxOffset += triangleCnt;
		}
	}

	Graphics2D::SetScissorRect(prevScissorRect);
	RenderImeWindow();
}

#include <cinttypes>
#include <string>
#ifdef _WIN32
#include <shellapi.h>
#endif
#include "../vender/imgui/imgui.h"
#include "../vender/imgui/imgui_impl_glfw.h"
#include "../vender/imgui/imgui_impl_opengl3.h"

namespace ImGui {
	inline void AddUnderLine(ImColor col_)
	{
		ImVec2 min = ImGui::GetItemRectMin();
		ImVec2 max = ImGui::GetItemRectMax();
		min.y = max.y;
		ImGui::GetWindowDrawList()->AddLine(min, max, col_, 1.0f);
	}

	/**
	*
	* Make a URL in Dear ImGui.
	*
	* @param const char* name_
	*
	*/
	inline void TextURL(const char* name_, const char* URL_, uint8_t SameLineBefore_ = 0, uint8_t SameLineAfter_ = 0)
	{
		if (1 == SameLineBefore_) { ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x); }
		ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered]);
		ImGui::Text(name_);
		ImGui::PopStyleColor();
		if (ImGui::IsItemHovered())
		{
			if (ImGui::IsMouseClicked(0))
			{
#ifdef _WIN32
				ShellExecuteA(NULL, "open", URL_, NULL, NULL, SW_SHOWNORMAL);
#elif defined(__APPLE__)
				system((std::string("open ") + std::string(URL_)).c_str());
#elif defined(__linux__)
				system((std::string("xdg-open ") + std::string(URL_)).c_str());
#endif
			}
			AddUnderLine(ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered]);
		}
		else
		{
			AddUnderLine(ImGui::GetStyle().Colors[ImGuiCol_Button]);
		}
		if (1 == SameLineAfter_) { ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x); }
	}
}
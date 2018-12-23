#pragma once
#include <map>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"
class ThemeManager
{
private:
	enum class Theme;
	Theme currentTheme = Theme::DEFAULT;
	void StyleColorsLight()
	{
		ImGuiStyle* style = &ImGui::GetStyle();
		ImVec4* colors = style->Colors;

		colors[ImGuiCol_Text] = AccentColour2;
		colors[ImGuiCol_TextDisabled] = SecondaryColour;
		colors[ImGuiCol_WindowBg] = PrimaryColour;
		colors[ImGuiCol_ChildBg] = PrimaryColour;
		colors[ImGuiCol_PopupBg] = PrimaryColour;
		colors[ImGuiCol_Border] = AccentColour1;
		colors[ImGuiCol_BorderShadow] = AccentColour3;
		colors[ImGuiCol_FrameBg] = AccentColour1;
		colors[ImGuiCol_FrameBgHovered] = PrimaryColour;
		colors[ImGuiCol_FrameBgActive] = SecondaryColour;
		colors[ImGuiCol_TitleBg] = TitleColour;
		colors[ImGuiCol_TitleBgActive] = SecondaryColour;
		colors[ImGuiCol_TitleBgCollapsed] = AccentColour1;
		colors[ImGuiCol_MenuBarBg] = TitleColour;
		colors[ImGuiCol_ScrollbarBg] = AccentColour1;
		colors[ImGuiCol_ScrollbarGrab] = SecondaryColour;
		colors[ImGuiCol_ScrollbarGrabHovered] = SecondaryColour;
		colors[ImGuiCol_ScrollbarGrabActive] = PrimaryColour;
		colors[ImGuiCol_CheckMark] = SecondaryColour;
		colors[ImGuiCol_SliderGrab] = AccentColour1;
		colors[ImGuiCol_SliderGrabActive] = AccentColour1;
		colors[ImGuiCol_Button] = SecondaryColour;
		colors[ImGuiCol_ButtonHovered] = PrimaryColour;
		colors[ImGuiCol_ButtonActive] = AccentColour1;
		colors[ImGuiCol_Header] = AccentColour1;
		colors[ImGuiCol_HeaderHovered] = ActiveColour1;
		colors[ImGuiCol_HeaderActive] = DisabledColour1;
		colors[ImGuiCol_Separator] = AccentColour2;
		colors[ImGuiCol_SeparatorHovered] = DisabledColour2;
		colors[ImGuiCol_SeparatorActive] = ActiveColour2;
		colors[ImGuiCol_ResizeGrip] = Sate1Colour;
		colors[ImGuiCol_ResizeGripHovered] = Sate2Colour;
		colors[ImGuiCol_ResizeGripActive] = Sate3Colour;
		colors[ImGuiCol_PlotLines] = Sate1Colour;
		colors[ImGuiCol_PlotLinesHovered] = Sate2Colour;
		colors[ImGuiCol_PlotHistogram] = Sate1Colour;
		colors[ImGuiCol_PlotHistogramHovered] = Sate2Colour;
		colors[ImGuiCol_TextSelectedBg] = Sate3Colour;
		colors[ImGuiCol_ModalWindowDarkening] = Sate3Colour;
		colors[ImGuiCol_DragDropTarget] = Sate3Colour;
	}
	void StyleColorsDark()
	{
		ImGuiStyle* style = &ImGui::GetStyle();
		ImVec4* colors = style->Colors;

		colors[ImGuiCol_Text] = AccentColour2;
		colors[ImGuiCol_TextDisabled] = SecondaryColour;
		colors[ImGuiCol_WindowBg] = PrimaryColour;
		colors[ImGuiCol_ChildBg] = PrimaryColour;
		colors[ImGuiCol_PopupBg] = PrimaryColour;
		colors[ImGuiCol_Border] = AccentColour1;
		colors[ImGuiCol_BorderShadow] = AccentColour3;
		colors[ImGuiCol_FrameBg] = AccentColour1;
		colors[ImGuiCol_FrameBgHovered] = PrimaryColour;
		colors[ImGuiCol_FrameBgActive] = SecondaryColour;
		colors[ImGuiCol_TitleBg] = TitleColour;
		colors[ImGuiCol_TitleBgActive] = SecondaryColour;
		colors[ImGuiCol_TitleBgCollapsed] = AccentColour1;
		colors[ImGuiCol_MenuBarBg] = TitleColour;
		colors[ImGuiCol_ScrollbarBg] = AccentColour1;
		colors[ImGuiCol_ScrollbarGrab] = SecondaryColour;
		colors[ImGuiCol_ScrollbarGrabHovered] = SecondaryColour;
		colors[ImGuiCol_ScrollbarGrabActive] = PrimaryColour;
		colors[ImGuiCol_CheckMark] = SecondaryColour;
		colors[ImGuiCol_SliderGrab] = AccentColour1;
		colors[ImGuiCol_SliderGrabActive] = AccentColour1;
		colors[ImGuiCol_Button] = SecondaryColour;
		colors[ImGuiCol_ButtonHovered] = PrimaryColour;
		colors[ImGuiCol_ButtonActive] = AccentColour1;
		colors[ImGuiCol_Header] = AccentColour1;
		colors[ImGuiCol_HeaderHovered] = ActiveColour1;
		colors[ImGuiCol_HeaderActive] = DisabledColour1;
		colors[ImGuiCol_Separator] = AccentColour2;
		colors[ImGuiCol_SeparatorHovered] = DisabledColour2;
		colors[ImGuiCol_SeparatorActive] = ActiveColour2;
		colors[ImGuiCol_ResizeGrip] = Sate1Colour;
		colors[ImGuiCol_ResizeGripHovered] = Sate2Colour;
		colors[ImGuiCol_ResizeGripActive] = Sate3Colour;
		colors[ImGuiCol_PlotLines] = Sate1Colour;
		colors[ImGuiCol_PlotLinesHovered] = Sate2Colour;
		colors[ImGuiCol_PlotHistogram] = Sate1Colour;
		colors[ImGuiCol_PlotHistogramHovered] = Sate2Colour;
		colors[ImGuiCol_TextSelectedBg] = Sate3Colour;
		colors[ImGuiCol_ModalWindowDarkening] = Sate3Colour;
		colors[ImGuiCol_DragDropTarget] = Sate3Colour;

	}
	void CustomColourImGuiTheme()
	{
		ImGuiStyle* style = &ImGui::GetStyle();
		ImVec4* colors = style->Colors;

		colors[ImGuiCol_Text] = AccentColour2;
		colors[ImGuiCol_TextDisabled] = SecondaryColour;
		colors[ImGuiCol_WindowBg] = PrimaryColour;
		colors[ImGuiCol_ChildBg] = PrimaryColour;
		colors[ImGuiCol_PopupBg] = PrimaryColour;
		colors[ImGuiCol_Border] = AccentColour1;
		colors[ImGuiCol_BorderShadow] = AccentColour3;
		colors[ImGuiCol_FrameBg] = AccentColour1;
		colors[ImGuiCol_FrameBgHovered] = PrimaryColour;
		colors[ImGuiCol_FrameBgActive] = SecondaryColour;
		colors[ImGuiCol_TitleBg] = TitleColour;
		colors[ImGuiCol_TitleBgActive] = SecondaryColour;
		colors[ImGuiCol_TitleBgCollapsed] = AccentColour1;
		colors[ImGuiCol_MenuBarBg] = TitleColour;
		colors[ImGuiCol_ScrollbarBg] = AccentColour1;
		colors[ImGuiCol_ScrollbarGrab] = SecondaryColour;
		colors[ImGuiCol_ScrollbarGrabHovered] = SecondaryColour;
		colors[ImGuiCol_ScrollbarGrabActive] = PrimaryColour;
		colors[ImGuiCol_CheckMark] = SecondaryColour;
		colors[ImGuiCol_SliderGrab] = AccentColour1;
		colors[ImGuiCol_SliderGrabActive] = AccentColour1;
		colors[ImGuiCol_Button] = SecondaryColour;
		colors[ImGuiCol_ButtonHovered] = PrimaryColour;
		colors[ImGuiCol_ButtonActive] = AccentColour1;
		colors[ImGuiCol_Header] = AccentColour1;
		colors[ImGuiCol_HeaderHovered] = ActiveColour1;
		colors[ImGuiCol_HeaderActive] = DisabledColour1;
		colors[ImGuiCol_Separator] = AccentColour2;
		colors[ImGuiCol_SeparatorHovered] = DisabledColour2;
		colors[ImGuiCol_SeparatorActive] = ActiveColour2;
		colors[ImGuiCol_ResizeGrip] = Sate1Colour;
		colors[ImGuiCol_ResizeGripHovered] = Sate2Colour;
		colors[ImGuiCol_ResizeGripActive] = Sate3Colour;
		colors[ImGuiCol_PlotLines] = Sate1Colour;
		colors[ImGuiCol_PlotLinesHovered] = Sate2Colour;
		colors[ImGuiCol_PlotHistogram] = Sate1Colour;
		colors[ImGuiCol_PlotHistogramHovered] = Sate2Colour;
		colors[ImGuiCol_TextSelectedBg] = Sate3Colour;
		colors[ImGuiCol_ModalWindowDarkening] = Sate3Colour;
		colors[ImGuiCol_DragDropTarget] = Sate3Colour;
	}
public:
	enum class Theme
	{
		DEFAULT, DARK, LIGHT, BLUE
	};
	ImVec4 PrimaryColour;
	//Applied on title and menu bar
	ImVec4 TitleColour;
	ImVec4 SecondaryColour;
	ImVec4 AccentColour1;
	//Applied on text and separator
	ImVec4 AccentColour2;
	//Applied on border shadow
	ImVec4 AccentColour3;
	ImVec4 ActiveColour1;
	ImVec4 ActiveColour2;
	ImVec4 DisabledColour1;
	ImVec4 DisabledColour2;
	ImVec4 Sate1Colour;
	ImVec4 Sate2Colour;
	ImVec4 Sate3Colour;

	void Init()
	{
		PrimaryColour = ImVec4(40 / 255.0f, 49 / 255.0f, 73.0f / 255.0f, 1.1f);
		TitleColour = ImVec4(30 / 255.0f, 39 / 255.0f, 63.0f / 255.0f, 1.1f);
		SecondaryColour = ImVec4(247 / 255.0f, 56 / 255.0f, 89 / 255.0f, 1.1f);
		AccentColour1 = ImVec4(64.0f / 255.0f, 75.0f / 255.0f, 105.0f / 255.0f, 1.1f);
		AccentColour2 = ImVec4(1, 1, 1, 1.1f);
		AccentColour3 = ImVec4(40 / 255.0f, 40 / 255.0f, 40 / 255.0f, 1.1f);
		ActiveColour1 = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
		DisabledColour1 = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		ActiveColour2 = ImVec4(0.14f, 0.44f, 0.80f, 1.00f);
		DisabledColour2 = ImVec4(0.14f, 0.44f, 0.80f, 0.78f);
		Sate1Colour = ImVec4(0.30f, 0.30f, 0.70f, 0.46f);
		Sate2Colour = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
		Sate3Colour = ImVec4(0.10f, 0.1f, 0.25f, 0.01f);
	}
	void EnableInBuiltTheme(Theme theme)
	{
		currentTheme = theme;
		switch (theme)
		{
		case Theme::DEFAULT:
			PrimaryColour = ImVec4(40 / 255.0f, 49 / 255.0f, 73.0f / 255.0f, 1.1f);
			TitleColour = ImVec4(30 / 255.0f, 39 / 255.0f, 63.0f / 255.0f, 1.1f);
			SecondaryColour = ImVec4(247 / 255.0f, 56 / 255.0f, 89 / 255.0f, 1.1f);
			AccentColour1 = ImVec4(64.0f / 255.0f, 75.0f / 255.0f, 105.0f / 255.0f, 1.1f);
			AccentColour2 = ImVec4(1, 1, 1, 1.1f);
			AccentColour3 = ImVec4(40 / 255.0f, 40 / 255.0f, 40 / 255.0f, 1.1f);
			ActiveColour1 = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
			DisabledColour1 = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
			ActiveColour2 = ImVec4(0.14f, 0.44f, 0.80f, 1.00f);
			DisabledColour2 = ImVec4(0.14f, 0.44f, 0.80f, 0.78f);
			Sate1Colour = ImVec4(0.30f, 0.30f, 0.70f, 0.46f);
			Sate2Colour = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
			Sate3Colour = ImVec4(0.0f, 0.0f, 0.0f, 0.6f);
			CustomColourImGuiTheme();
			break;
		case Theme::DARK:
			PrimaryColour = ImVec4(0.2f,0.2f,0.2f, 1.1f);
			TitleColour = ImVec4(0.13f, 0.13f, 0.13f, 1.1f);
			SecondaryColour = ImVec4(0.3f ,0.3f, 0.3f, 1.1f);
			AccentColour1 = ImVec4(0.15f, 0.15f, 0.15f, 1.1f);
			AccentColour2 = ImVec4(1, 1, 1, 1.1f);
			AccentColour3 = ImVec4(0.9f,0.9f,0.9f, 1.1f);
			ActiveColour1 = ImVec4(0.93f,0.93f,0.93f, 0.80f);
			DisabledColour1 = ImVec4(0.6f, 0.6f, 0.6f, 1.00f);
			ActiveColour2 = ImVec4(0.6f, 0.6f, 0.6f, 1.00f);
			DisabledColour2 = ImVec4(0.6f, 0.6f, 0.6f, 0.78f);
			Sate1Colour = ImVec4(0.80f, 0.80f, 0.80f, 0.46f);
			Sate2Colour = ImVec4(0.8f, 0.8f, 0.8f, 0.67f);
			Sate3Colour = ImVec4(0.0f, 0.0f, 0.0f, 0.6f);
			StyleColorsDark();
			break;
		case Theme::LIGHT:
			PrimaryColour = ImVec4(0.9f, 0.9f, 0.95f, 1.1f);
			TitleColour = ImVec4(0.93f, 0.93f, 0.95f, 1.1f);
			SecondaryColour = ImVec4(0.8f, 0.8f, 0.8f, 1.1f);
			AccentColour1 = ImVec4(0.65f, 0.65f, 0.65f, 1.1f);
			AccentColour2 = ImVec4(0.15f, 0.15f, 0.15f, 1.1f);
			AccentColour3 = ImVec4(0.2f, 0.2f, 0.2f, 1.1f);
			ActiveColour1 = ImVec4(0.75f, 0.75f, 0.77f, 0.80f);
			DisabledColour1 = ImVec4(0.6f, 0.6f, 0.6f, 1.00f);
			ActiveColour2 = ImVec4(0.6f, 0.6f, 0.6f, 1.00f);
			DisabledColour2 = ImVec4(0.6f, 0.6f, 0.6f, 0.78f);
			Sate1Colour = ImVec4(0.50f, 0.50f, 0.50f, 0.46f);
			Sate2Colour = ImVec4(0.8f, 0.8f, 0.8f, 0.67f);
			Sate3Colour = ImVec4(0.0f, 0.0f, 0.0f, 0.6f);
			StyleColorsLight();
			break;
		case Theme::BLUE:
			ImGui::StyleColorsClassic();
			break;
		default:
			break;
		}
	}
};
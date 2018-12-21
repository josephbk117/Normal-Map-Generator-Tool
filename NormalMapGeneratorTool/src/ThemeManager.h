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

	void StyleColorsDark()
	{
		ImGuiStyle* style = &ImGui::GetStyle();
		ImVec4* colors = style->Colors;

		colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
		colors[ImGuiCol_ChildBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
		colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.2f, 0.2f, 0.54f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.4f, 0.4f, 0.4f, 0.40f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.3f, 0.3f, 0.3f, 0.67f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.7f, 0.7f, 0.7f, 0.40f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.8f, 0.8f, 0.8f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.6f, 0.6f, 0.68f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.6f, 0.6f, 0.6f, 0.31f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.7f, 0.7f, 0.7f, 0.80f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.7f, 0.7f, 0.7f, 1.00f);
		colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
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
		Sate3Colour = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
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
			Sate3Colour = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
			CustomColourImGuiTheme();
			break;
		case Theme::DARK:
			PrimaryColour = ImVec4(0.2f,0.2f,0.2f, 1.1f);
			TitleColour = ImVec4(0.7f,0.7f,0.7f, 1.1f);
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
			Sate3Colour = ImVec4(0.8f, 0.8f, 0.8f, 0.95f);
			StyleColorsDark();
			break;
		case Theme::LIGHT:
			PrimaryColour = ImVec4(0.62f, 0.62f, 0.82f, 1.1f);
			TitleColour = ImVec4(0.8f, 0.8f, 0.97f, 1.1f);
			SecondaryColour = ImVec4(0.75f, 0.75f, 0.83f, 1.1f);
			AccentColour1 = ImVec4(0.35f, 0.85f, 0.85f, 1.1f);
			AccentColour2 = ImVec4(1, 1, 1, 1.1f);
			AccentColour3 = ImVec4(0.9f, 0.9f, 0.9f, 1.1f);
			ActiveColour1 = ImVec4(0.93f, 0.93f, 0.93f, 0.80f);
			DisabledColour1 = ImVec4(0.6f, 0.6f, 0.6f, 1.00f);
			ActiveColour2 = ImVec4(0.6f, 0.6f, 0.6f, 1.00f);
			DisabledColour2 = ImVec4(0.6f, 0.6f, 0.6f, 0.78f);
			Sate1Colour = ImVec4(0.80f, 0.80f, 0.80f, 0.46f);
			Sate2Colour = ImVec4(0.8f, 0.8f, 0.8f, 0.67f);
			Sate3Colour = ImVec4(0.8f, 0.8f, 0.8f, 0.95f);
			ImGui::StyleColorsLight();
			break;
		case Theme::BLUE:
			ImGui::StyleColorsClassic();
			break;
		default:
			break;
		}
	}
};
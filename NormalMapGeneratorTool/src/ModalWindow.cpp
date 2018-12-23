#include "ModalWindow.h"


ModalWindow::ModalWindow()
{
}
void ModalWindow::setTitleFont(ImFont* titleFont)
{
	this->titleFont = titleFont;
}
void ModalWindow::setModalDialog(const std::string & title, const std::string & content)
{
	this->title = title;
	this->content = content;
	shouldShow = true;
}
void ModalWindow::display()
{
	if (!shouldShow)
		return;
	ImGui::SetNextWindowSize(ImVec2(400, 200));
	ImGui::OpenPopup("Popup");
	if (ImGui::BeginPopupModal("Popup", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove))
	{
		ImGui::PushFont(titleFont);
		float xval = ImGui::GetContentRegionAvailWidth()*0.5f - ImGui::CalcTextSize(title.c_str()).x*0.5f;
		ImGui::Indent(xval);
		ImGui::Text(title.c_str());
		ImGui::PopFont();
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Indent(-xval);
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();
		xval = ImGui::GetContentRegionAvailWidth()*0.5f - ImGui::CalcTextSize(content.c_str()).x*0.5f;

		ImGui::TextWrapped(content.c_str());
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
		if (ImGui::Button("##Gap", ImVec2(ImGui::GetContentRegionAvailWidth(), ImGui::GetContentRegionAvail().y - 40)));
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		if (ImGui::Button("Close", ImVec2(ImGui::GetContentRegionAvailWidth(), 30)))
		{
			shouldShow = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

ModalWindow::~ModalWindow()
{
}

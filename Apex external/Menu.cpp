#include "overlay.hpp"
#include "config.h"
#include <format>

class GradientLine {
public:

	static bool Render(ImVec2 size)
	{
		static ImColor gradient_colors[] =
		{

			ImColor(255, 0, 0),

			ImColor(255,255,0),

			ImColor(0, 255, 0),

			ImColor(0, 255, 255),

			ImColor(0, 0, 255),

			ImColor(255, 0, 255),

			ImColor(255, 0, 0)
		};

		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2      screen_pos = ImGui::GetCursorScreenPos();

		static int pos = 0;

		if (size.x - pos == 0)
			pos = 0;
		else
			pos++;

		for (int i = 0; i < 6; ++i)
		{
			ImVec2 item_spacing = ImGui::GetStyle().ItemSpacing;

			auto render = [&](int displacement)
				{
					draw_list->AddRectFilledMultiColor
					(
						ImVec2((screen_pos.x - item_spacing.x - displacement) + (i) * (size.x / 6), (screen_pos.y - item_spacing.y)),
						ImVec2((screen_pos.x - item_spacing.x + (item_spacing.x * 2) - displacement) + (i + 1) * (size.x / 6), (screen_pos.y - item_spacing.y) + (size.y)),

						gradient_colors[i], gradient_colors[i + 1], gradient_colors[i + 1], gradient_colors[i]
					);
				};

			render((pos)); render((pos - size.x));
		}
		return true;
	}
};

void Overlay::Render()
{

	ImGui::GetForegroundDrawList()->AddRectFilled(ImGui::GetIO().MousePos, ImVec2(ImGui::GetIO().MousePos.x + 5.f,
		ImGui::GetIO().MousePos.y + 5.f), ImColor(255, 255, 255));

	ImGuiStyle* style = &ImGui::GetStyle();

	ImVec4* colors = style->Colors;

	style->Colors[ImGuiCol_Text] = ImColor(255, 255, 255, 255);
	style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.0f, 0.0263f, 0.0357f, 1.00f);
	style->Colors[ImGuiCol_WindowBg] = ImColor(25, 25, 25);
	style->Colors[ImGuiCol_ChildBg] = ImColor(20, 20, 20);
	style->Colors[ImGuiCol_PopupBg] = ImVec4(0.080f, 0.080f, 0.080f, 0.940f);
	style->Colors[ImGuiCol_Border] = ImColor(57, 57, 57);
	style->Colors[ImGuiCol_BorderShadow] = ImColor(1, 1, 1);
	style->Colors[ImGuiCol_FrameBg] = ImColor(40, 40, 40);
	style->Colors[ImGuiCol_FrameBgHovered] = ImColor(40, 40, 40);
	style->Colors[ImGuiCol_FrameBgActive] = ImColor(40, 40, 40);
	style->Colors[ImGuiCol_TitleBg] = ImVec4(0.102f, 0.090f, 0.122f, 1.000f);
	style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.102f, 0.090f, 0.122f, 1.000f);
	style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.102f, 0.090f, 0.122f, 1.000f);
	style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.0f, 0.263f, 0.357f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);

	style->Colors[ImGuiCol_SliderGrab] = ImColor(111, 0, 255);
	style->Colors[ImGuiCol_SliderGrabActive] = ImColor(111, 0, 255);
	style->Colors[ImGuiCol_Button] = ImColor(20, 20, 20);
	style->Colors[ImGuiCol_ButtonHovered] = ImColor(20, 20, 20);
	style->Colors[ImGuiCol_ButtonActive] = ImColor(20, 20, 20);
	style->Colors[ImGuiCol_Header] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);

	style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style->Colors[ImGuiCol_PlotLines] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);

	style->Colors[ImGuiCol_Separator] = ImColor(105, 0, 255);

	style->WindowRounding = 0;

	static int tabb = 0;

	ImGui::SetNextWindowSize(ImVec2(450.000f, 550.000f), ImGuiCond_Once);

	ImGui::Begin(("cheat"), 0, ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar);

	GradientLine::Render(ImVec2(ImGui::GetContentRegionAvail().x * 2, 3));
	ImGui::BeginChild(("##backround"), ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar);
	{
		if (ImGui::Button("aimbot", ImVec2(77, 20)))
		{
			tabb = 1;
		}

		ImGui::SameLine();

		if (ImGui::Button("visuals", ImVec2(77, 20)))
		{
			tabb = 2;
		}

		ImGui::SameLine();

		if (ImGui::Button("misc", ImVec2(77, 20)))
		{
			tabb = 3;
		}

		ImGui::SameLine();

		if (ImGui::Button("config", ImVec2(77, 20)))
		{
			tabb = 4;
		}

		if (tabb == 1)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ImColor(111, 0, 255, 255)));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(ImColor(111, 0, 255, 255)));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(ImColor(111, 0, 255, 255)));

			ImGui::Button(" ", ImVec2(77, 3));

			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();

		}
		else
		{

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ImColor(30, 30, 30, 255)));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(ImColor(30, 30, 30, 255)));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(ImColor(30, 30, 30, 255)));

			ImGui::Button(" ", ImVec2(77, 3));

			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
		}

		if (tabb == 2)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ImColor(111, 0, 255, 255)));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(ImColor(111, 0, 255, 255)));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(ImColor(111, 0, 255, 255)));

			ImGui::SameLine();

			ImGui::Button(" ", ImVec2(77, 3));

			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();

		}
		else
		{

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ImColor(30, 30, 30, 255)));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(ImColor(30, 30, 30, 255)));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(ImColor(30, 30, 30, 255)));

			ImGui::SameLine();

			ImGui::Button(" ", ImVec2(77, 3));

			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
		}

		if (tabb == 3)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ImColor(111, 0, 255, 255)));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(ImColor(111, 0, 255, 255)));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(ImColor(111, 0, 255, 255)));

			ImGui::SameLine();

			ImGui::Button(" ", ImVec2(77, 3));

			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();

		}
		else
		{

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ImColor(30, 30, 30, 255)));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(ImColor(30, 30, 30, 255)));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(ImColor(30, 30, 30, 255)));

			ImGui::SameLine();

			ImGui::Button(" ", ImVec2(77, 3));

			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
		}

		if (tabb == 5)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ImColor(111, 0, 255, 255)));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(ImColor(111, 0, 255, 255)));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(ImColor(111, 0, 255, 255)));

			ImGui::SameLine();

			ImGui::Button(" ", ImVec2(77, 3));

			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();

		}
		else
		{

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ImColor(30, 30, 30, 255)));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(ImColor(30, 30, 30, 255)));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(ImColor(30, 30, 30, 255)));

			ImGui::SameLine();

			ImGui::Button(" ", ImVec2(77, 3));

			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
		}

		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(ImColor(15, 15, 15, 255)));

		ImGui::BeginChild(("##backro3und"), ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar);
		{
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::SameLine();

			if (tabb == 1)
			{
				ImGui::BeginChild(("##tab1left"), ImVec2(189, 455), true, ImGuiWindowFlags_NoScrollbar);
				{

					ImGui::Checkbox("Aimbot", &config::aimbot_enabled);
					ImGui::Checkbox("X Only", &config::X_Only);
					ImGui::Checkbox("Visible Check", &config::VisCheck);

					ImGui::EndChild();
				}

				ImGui::SameLine();

				ImGui::BeginChild(("##tab1right"), ImVec2(189, 455), true, ImGuiWindowFlags_NoScrollbar);
				{
					ImGui::Text(std::format("FOV: {:.0f}", config::aimbot_fov).c_str());
					ImGui::SliderFloat("##FOV", &config::aimbot_fov, 1.f, 1000.f, "%.0f");

					ImGui::Text(std::format("Smoothing: {:.1f}", config::aimbot_smooth).c_str());
					ImGui::SliderFloat("##Smoothing", &config::aimbot_smooth, 1.f, 20.f, "%.1f");

					ImGui::EndChild();
				}
			}

			if (tabb == 2)
			{
				ImGui::BeginChild(("##tab2left"), ImVec2(189, 455), true, ImGuiWindowFlags_NoScrollbar);
				{
					ImGui::Checkbox("Player Box", &config::PlayerBox);
					if (config::PlayerBox)
					{
						ImGui::Checkbox("Corner Box", &config::PlayerCornorBox);
					}
					ImGui::Checkbox("Player Skeleton", &config::PlayerSkeleton);
					ImGui::Checkbox("Health Bar", &config::playerhealthbar);
					ImGui::Checkbox("Snapline", &config::PlayerSnapline);

					ImGui::EndChild();
				}

				ImGui::SameLine();

				ImGui::BeginChild(("##tab2right"), ImVec2(189, 455), true, ImGuiWindowFlags_NoScrollbar);
				{

					ImGui::Text(std::format("Max Distance: {:.1f}", config::MaxDistance).c_str());
					ImGui::SliderFloat("##Max Distance", &config::MaxDistance, 1.f, 1000.f);

					ImGui::EndChild();
				}
			}

			if (tabb == 3)
			{
				ImGui::BeginChild(("##tab3left"), ImVec2(189, 455), true, ImGuiWindowFlags_NoScrollbar);
				{

					ImGui::Checkbox("Team Check", &config::Team_Check);

					ImGui::EndChild();
				}

				ImGui::SameLine();

				ImGui::BeginChild(("##tab3right"), ImVec2(189, 455), true, ImGuiWindowFlags_NoScrollbar);
				{

					ImGui::EndChild();
				}
			}

			if (tabb == 4)
			{
				ImGui::BeginChild(("##tab4left"), ImVec2(189, 455), true, ImGuiWindowFlags_NoScrollbar);
				{

					ImGui::EndChild();
				}

				ImGui::SameLine();

				ImGui::BeginChild(("##tab4right"), ImVec2(189, 455), true, ImGuiWindowFlags_NoScrollbar);
				{

					ImGui::EndChild();
				}
			}

			if (tabb == 5)
			{
				ImGui::BeginChild(("##tab5left"), ImVec2(189, 455), true, ImGuiWindowFlags_NoScrollbar);
				{

					ImGui::EndChild();
				}

				ImGui::SameLine();

				ImGui::BeginChild(("##tab5right"), ImVec2(189, 455), true, ImGuiWindowFlags_NoScrollbar);
				{

					ImGui::EndChild();
				}
			}

			ImGui::EndChild();

		}

		ImGui::PopStyleColor();

		ImGui::EndChild();
	}

	ImGui::End();

}
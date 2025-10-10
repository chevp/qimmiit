/**
 * Tool Menu Integration - Helper for integrating tool launcher into ImGui overlays
 *
 * Usage in your renderer:
 *
 * 1. Initialize tool launcher once at startup:
 *    toollaunch::g_toolLauncher = std::make_unique<toollaunch::ToolLauncher>();
 *    toollaunch::g_toolLauncher->initialize();
 *
 * 2. Add to your ImGui rendering code (in main menu bar):
 *    if (ui->beginMainMenuBar()) {
 *        // ... other menus ...
 *        toollaunch::renderToolsMenu(ui);
 *        ui->endMainMenuBar();
 *    }
 *
 * 3. Shutdown at application exit:
 *    toollaunch::g_toolLauncher->shutdown();
 */

#pragma once

#include <coregfx/overlay/tool_launcher.hpp>
#include <coregfx/overlay/overlay_imgui.hpp>

namespace toollaunch {

    /**
     * Render the Tools menu in an existing menu bar
     *
     * @param ui The ImGui UI instance (overlayimgui::UI)
     */
    inline void renderToolsMenu(overlayimgui::UI* ui) {
        if (!g_toolLauncher) {
            return;
        }

        g_toolLauncher->renderToolsMenu();
    }

    /**
     * Render tools as a standalone window (alternative to menu bar)
     */
    inline void renderToolsWindow() {
        if (!g_toolLauncher) {
            return;
        }

        ImGui::Begin("Tool Launcher", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::Text("Arctic Development Tools");
        ImGui::Separator();

        auto tools = g_toolLauncher->getAllTools();
        for (const auto& tool : tools) {
            bool running = g_toolLauncher->isToolRunning(tool.id);

            std::string label = tool.icon + " " + tool.displayName;

            if (running) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
                if (ImGui::Button((label + " (Running)").c_str(), ImVec2(200, 0))) {
                    g_toolLauncher->focusTool(tool.id);
                }
                ImGui::PopStyleColor();

                ImGui::SameLine();
                if (ImGui::SmallButton(("Stop##" + tool.id).c_str())) {
                    g_toolLauncher->stopTool(tool.id);
                }
            } else {
                if (ImGui::Button(label.c_str(), ImVec2(200, 0))) {
                    g_toolLauncher->launchTool(tool.id);
                }
            }
        }

        ImGui::Separator();

        if (ImGui::Button("Stop All Tools", ImVec2(200, 0))) {
            g_toolLauncher->stopAllTools();
        }

        ImGui::End();
    }

} // namespace toollaunch

#include "AgentModel/UIRenderer.hpp"
#include "AgentModel/SimulationEngine.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <filesystem>

#include "implot_internal.h"


namespace AgentModel::UI {

    void UIRenderer::render_setup(Engine::SimulationEngine& simulation_engine) {

        const ImGuiIO& io = ImGui::GetIO();

        const float window_width = 500.0f;
        const float window_height = 300.0f;

        const ImVec2 center_pos(
            (io.DisplaySize.x - window_width) * 0.5f,
            (io.DisplaySize.y - window_height) * 0.5f
        );

        ImGui::SetNextWindowPos(center_pos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(window_width, window_height), ImGuiCond_Always);

        if (ImGui::Begin("Simulation Setup", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {

            ImGui::TextColored(ImVec4(0.26f, 0.59f, 0.98f, 1.0f), "Initialise Brock-Hommes Stochastic Model");
            ImGui::Separator();
            ImGui::Dummy(ImVec2(0.0f, 20.0f));

            ImGui::Text("Select the size of the Agent Population:");
            ImGui::PushItemWidth(-1);
            ImGui::SliderInt("##AgentCount", &selected_agent_count, 10000, 20000000, "%d actors");
            ImGui::PopItemWidth();

            ImGui::Dummy(ImVec2(0.0f, 40.0f));

            if (ImGui::Button("START", ImVec2(-1, 50))) {

                current_app_state = AppState::Simulation;
                simulation_engine.simulation_start(selected_agent_count);
                /*if (on_start_engine) {

                    on_start_engine(static_cast<uint32_t>(selected_agent_count));

                }*/

            }

        }

        ImGui::End();

    }

    bool UIRenderer::init(const int width, const int height, const char* title) {

        if (!glfwInit()) return false;

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

        window = glfwCreateWindow(width, height, title, nullptr, nullptr);

        if (!window) {

            glfwTerminate();
            return false;

        }

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImPlot::CreateContext();

        const ImGuiIO& io = ImGui::GetIO();

        if (std::filesystem::exists("assets/JetBrainsMono-Regular.ttf")) {

            const float font_size = 20.0f;
            io.Fonts->AddFontFromFileTTF("assets/JetBrainsMono-Regular.ttf", font_size);

        } else {

            std::cerr << "Runtime Warning: Could not find assets/JetBrainsMono-Regular.ttf. Using default." << std::endl;
            io.Fonts->AddFontDefault();

        }

        set_theme();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 130");

        return true;

    }

void UIRenderer::set_theme() {

        // ==========================================
        // 1. Window properties
        // ==========================================
        auto& style = ImGui::GetStyle();
        auto* colors = style.Colors;

        // Adjusting corners rounding
        style.WindowRounding    = 2.0f;
        style.FrameRounding     = 2.0f;
        style.PopupRounding     = 2.0f;
        style.GrabRounding      = 2.0f;
        style.ScrollbarRounding = 2.0f;

        // Setting padding
        style.FramePadding      = ImVec2(10, 6);
        style.WindowPadding     = ImVec2(14, 14);
        style.ItemSpacing       = ImVec2(10, 8);

        // Border sizes
        style.WindowBorderSize  = 1.0f;
        style.FrameBorderSize   = 1.0f;
        style.PopupBorderSize   = 1.0f;

        // ==========================================
        // 2. Dark colour palette
        // ==========================================
        colors[ImGuiCol_WindowBg]               = ImVec4(0.04f, 0.05f, 0.06f, 1.00f);
        colors[ImGuiCol_ChildBg]                = ImVec4(0.06f, 0.07f, 0.08f, 1.00f);
        colors[ImGuiCol_PopupBg]                = ImVec4(0.06f, 0.07f, 0.08f, 1.00f);

        colors[ImGuiCol_FrameBg]                = ImVec4(0.10f, 0.12f, 0.15f, 1.00f);
        colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.14f, 0.16f, 0.20f, 1.00f);
        colors[ImGuiCol_FrameBgActive]          = ImVec4(0.18f, 0.21f, 0.26f, 1.00f);

        colors[ImGuiCol_TitleBg]                = ImVec4(0.04f, 0.05f, 0.06f, 1.00f);
        colors[ImGuiCol_TitleBgActive]          = ImVec4(0.07f, 0.09f, 0.12f, 1.00f);

        colors[ImGuiCol_Button]                 = ImVec4(0.12f, 0.35f, 0.58f, 1.00f);
        colors[ImGuiCol_ButtonHovered]          = ImVec4(0.17f, 0.44f, 0.70f, 1.00f);
        colors[ImGuiCol_ButtonActive]           = ImVec4(0.10f, 0.28f, 0.46f, 1.00f);

        colors[ImGuiCol_SliderGrab]             = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
        colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.35f, 0.65f, 1.00f, 1.00f);
        colors[ImGuiCol_CheckMark]              = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);

        colors[ImGuiCol_Text]                   = ImVec4(0.85f, 0.87f, 0.91f, 1.00f);
        colors[ImGuiCol_TextDisabled]           = ImVec4(0.40f, 0.42f, 0.46f, 1.00f);

        colors[ImGuiCol_Border]                 = ImVec4(0.15f, 0.17f, 0.20f, 0.60f);
        colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

        // ==========================================
        // 3. ImPlot chart style
        // ==========================================

        auto& plot_style = ImPlot::GetStyle();
        plot_style.LineWeight       = 1.5f;
        plot_style.MinorAlpha       = 0.15f;
        plot_style.PlotPadding      = ImVec2(12, 12);

        plot_style.Colors[ImPlotCol_PlotBg]     = ImVec4(0.06f, 0.07f, 0.08f, 1.00f);
        plot_style.Colors[ImPlotCol_PlotBorder] = ImVec4(0.15f, 0.17f, 0.20f, 1.00f);

        plot_style.Colors[ImPlotCol_AxisGrid]   = ImVec4(0.15f, 0.17f, 0.20f, 0.40f);
        plot_style.Colors[ImPlotCol_Line]       = ImVec4(0.24f, 0.52f, 0.88f, 0.50f);
        plot_style.Colors[ImPlotCol_Crosshairs] = ImVec4(0.85f, 0.87f, 0.91f, 0.60f);

    }

    // Candle drawing engine
    void UIRenderer::draw_candles(const char* id, const double* x, const double* open, const double* high, const double* low, const double* close, const int count, const double width) {

        if (ImPlot::BeginItem(id)) {

            ImDrawList* draw_list = ImPlot::GetPlotDrawList();

            for (int i = 0; i < count; ++i) {

                const ImU32 color = (close[i] >= open[i]) ? IM_COL32(8, 153, 129, 255) : IM_COL32(242, 54, 69, 255);

                ImVec2 high_px = ImPlot::PlotToPixels(ImPlotPoint(x[i], high[i]));
                ImVec2 low_px  = ImPlot::PlotToPixels(ImPlotPoint(x[i], low[i]));

                const double top = std::max(open[i], close[i]);
                const double bottom = std::min(open[i], close[i]);

                ImVec2 box_min = ImPlot::PlotToPixels(ImPlotPoint(x[i] - width * 0.4, top));
                ImVec2 box_max = ImPlot::PlotToPixels(ImPlotPoint(x[i] + width * 0.4, bottom));

                draw_list->AddLine(high_px, low_px, color, 1.5f);

                if (std::abs(box_min.y - box_max.y) < 1.0f) {

                    box_max.y = box_min.y + 1.0f;

                }

                draw_list->AddRectFilled(box_min, box_max, color);

            }

            ImPlot::EndItem();

        }

    }

    void UIRenderer::process_ticks(Engine::SimulationBridge& bridge) {

        while (const auto tick = bridge.buffer.pop()) {

            double price = tick->x_t;

            if (!is_candle_active) {

                active_candle.open = price;
                active_candle.high = price;
                active_candle.low = price;
                active_candle.close = price;
                active_candle.step = static_cast<double>(candles.size() + 1);
                is_candle_active = true;
                ticks_processed = 1;

            } else {

                active_candle.high = std::max(active_candle.high, price);
                active_candle.low = std::min(active_candle.low, price);
                active_candle.close = price;
                ticks_processed++;

            }

            if (ticks_processed >= ticks_per_candle) {

                candles.push_back(active_candle);
                is_candle_active = false;
                ticks_processed = 0;

            }
        }

        if (candles.size() > 5000) {

            candles.erase(candles.begin(), candles.begin() + 2500);

        }

    }

void UIRenderer::render_simulation(Engine::MarketWorld* market) const {

        const ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("Dashboard", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);

        ImGui::Text("History Size: %zu candles", candles.size());
        ImGui::Separator();

        // ==========================================
        // 1. InputFloat - on-the-run parameter modifying
        // ==========================================
        if (market != nullptr) {

            auto atomic_input = [](const char* label, std::atomic<double>& param, const float step) {

                float val = static_cast<float>(param.load(std::memory_order_relaxed));
                if (ImGui::InputFloat(label, &val, step, step * 10.0f, "%.3f")) {

                    param.store(val, std::memory_order_relaxed);

                }

            };

            ImGui::PushItemWidth(150.0f);

            ImGui::TextColored(ImVec4(0.26f, 0.59f, 0.98f, 1.0f), "Market Dynamic Parameters:");

            // Table of model parameter modifiers
            if (ImGui::BeginTable("MarketParamsTable", 2)) {

                ImGui::TableNextColumn();
                atomic_input("Choice Intensity (Beta)", market->choice_intensity, 0.1f);

                ImGui::TableNextColumn();
                atomic_input("Trend Following (g)", market->trend_following_intensity, 0.1f);

                ImGui::TableNextColumn();
                atomic_input("Risk-Free Rate (R)", market->discounter, 0.005f);

                ImGui::TableNextColumn();
                atomic_input("Fund. Cost (C_f)", market->fundamentalist_cost, 0.1f);

                ImGui::EndTable();

            }

            ImGui::Dummy(ImVec2(0, 5));

            ImGui::TextColored(ImVec4(0.98f, 0.40f, 0.26f, 1.0f), "Stochastic Noise Controls:");

            // Table of model parameter modifiers for random noise
            if (ImGui::BeginTable("NoiseParamsTable", 2)) {

                ImGui::TableNextColumn();
                atomic_input("T-Student DoF (Tails)", market->noise_degrees_of_freedom, 1.0f);

                ImGui::TableNextColumn();
                atomic_input("Noise Scale", market->noise_scale, 0.01f);

                ImGui::EndTable();
            }

            ImGui::PopItemWidth();
            ImGui::Separator();

        }

        double x_min = 0.0;
        auto x_max = static_cast<double>(max_visible_candles);

        double y_min = -5.0;
        double y_max = 5.0;

        size_t render_count = std::min(candles.size(), max_visible_candles);

        if (!candles.empty()) {

            const auto last_index = static_cast<double>(candles.size());

            if (last_index > max_visible_candles) {

                x_min = last_index - max_visible_candles;
                x_max = last_index;

            }

            if (render_count > 0) {

                const size_t start_idx = candles.size() - render_count;

                y_min = candles[start_idx].low;
                y_max = candles[start_idx].high;

                for (size_t i = 1; i < render_count; ++i) {

                    if (candles[start_idx + i].low < y_min) y_min = candles[start_idx + i].low;
                    if (candles[start_idx + i].high > y_max) y_max = candles[start_idx + i].high;

                }

                double padding = (y_max - y_min) * 0.1;
                if (padding == 0.0) padding = 1.0;

                y_min -= padding;
                y_max += padding;

            }

        }

        if (ImPlot::BeginPlot("Stochastic Brock-Hommes Model", ImVec2(-1, -1))) {

            ImPlot::SetupAxisLimits(ImAxis_X1, x_min, x_max, ImGuiCond_Always);
            ImPlot::SetupAxis(ImAxis_X1, "Timeline (1 candle = 15 ticks)");

            ImPlot::SetupAxisLimits(ImAxis_Y1, y_min, y_max, ImGuiCond_Always);
            ImPlot::SetupAxis(ImAxis_Y1, "Deviation from the Fundamental Price (x_t)");

            if (render_count > 0) {

                const size_t start_idx = candles.size() - render_count;

                std::vector<double> x_vals(render_count);
                std::vector<double> open_vals(render_count);
                std::vector<double> high_vals(render_count);
                std::vector<double> low_vals(render_count);
                std::vector<double> close_vals(render_count);

                for (size_t i = 0; i < render_count; ++i) {

                    const auto&[step, open, high, low, close] = candles[start_idx + i];
                    x_vals[i]     = step;
                    open_vals[i]  = open;
                    high_vals[i]  = high;
                    low_vals[i]   = low;
                    close_vals[i] = close;

                }

                draw_candles("Market Frame", x_vals.data(), open_vals.data(), high_vals.data(), low_vals.data(), close_vals.data(), static_cast<int>(render_count), 0.57);
            }

            ImPlot::EndPlot();

        }

        ImGui::End();
    }

    //void UIRenderer::run_loop(Engine::SimulationBridge& bridge, Engine::MarketWorld*& market_ptr) {
    void UIRenderer::run_loop(Engine::SimulationEngine& simulation_engine) {

        while (!glfwWindowShouldClose(window)) {

            glfwPollEvents();

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            if (current_app_state == AppState::Setup) {

                render_setup(simulation_engine);

            } else {

                process_ticks(simulation_engine.get_bridge());
                render_simulation(simulation_engine.get_market());

            }

            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(0.08f, 0.09f, 0.10f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window);

        }

    }

    void UIRenderer::shutdown() const {

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();

        if (window) {

            glfwDestroyWindow(window);

        }

        glfwTerminate();

    }

}
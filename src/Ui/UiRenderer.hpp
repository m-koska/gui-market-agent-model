#pragma once

#include <functional>
#include <vector>

#include <GLFW/glfw3.h>

#include "../Utils/SimulationBridge.hpp"

namespace Model {
	struct MarketWorld;
}

namespace Ui {

	enum class AppState : uint8_t {
		Setup,
		Simulation
	};

	class UiRenderer {

	private:

		GLFWwindow* window = nullptr;
		std::vector<Utils::CandleData> candles;

		bool is_candle_active = false;
		Utils::CandleData active_candle{};
		size_t ticks_processed = 0;

		const size_t ticks_per_candle = 15;
		const size_t max_visible_candles = 60;

		AppState current_app_state = AppState::Setup;
		int selected_agent_count = 1'000'000;

		void set_theme();

		static void draw_candles(const char* id, const double* x, const double* open, const double* high, const double* low, const double* close, int count, double width);
		void process_ticks(Utils::SimulationBridge& bridge);

		void render_setup();
		void render_simulation(Model::MarketWorld* market) const;

	public:

		UiRenderer() = default;
		~UiRenderer() = default;

		std::function<void(uint32_t)> on_start_engine;

		bool init(int width, int height, const char* title);
		void run_loop(Utils::SimulationBridge& bridge, Model::MarketWorld*& market_ptr);
		void shutdown() const;

	};

}

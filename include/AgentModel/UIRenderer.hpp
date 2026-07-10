#pragma once

#include "ModelStructures.hpp"
#include "SimulationBridge.hpp"

#include <functional>
#include <vector>

#include <GLFW/glfw3.h>

namespace AgentModel::Model {
	struct MarketWorld;
}

namespace AgentModel::UI {

	enum class AppState : uint8_t {
		Setup,
		Simulation
	};

	class UIRenderer {

	private:

		GLFWwindow* window = nullptr;
		std::vector<Engine::CandleData> candles;

		bool is_candle_active = false;
		Engine::CandleData active_candle{};
		size_t ticks_processed = 0;

		const size_t ticks_per_candle = 15;
		const size_t max_visible_candles = 60;

		AppState current_app_state = AppState::Setup;
		int selected_agent_count = 1'000'000;

		void set_theme();

		static void draw_candles(const char* id, const double* x, const double* open, const double* high, const double* low, const double* close, int count, double width);
		void process_ticks(Engine::SimulationBridge& bridge);

		void render_setup();
		void render_simulation(Engine::MarketWorld* market) const;

	public:

		std::function<void(uint32_t)> on_start_engine;

		bool init(int width, int height, const char* title);
		void run_loop(Engine::SimulationBridge& bridge, Engine::MarketWorld*& market_ptr);
		void shutdown() const;

	};

}

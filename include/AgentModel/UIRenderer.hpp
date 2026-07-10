#pragma once

#include "ModelStructures.hpp"
#include "SimulationEngine.hpp"

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

		void render_setup(Engine::SimulationEngine& simulation_engine);
		void render_simulation(Engine::MarketWorld* market) const;

	public:

		bool init(int width, int height, const char* title);
		void run_loop(Engine::SimulationEngine& simulation_engine);
		void shutdown() const;

	};

}

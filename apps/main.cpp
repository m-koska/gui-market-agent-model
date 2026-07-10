#include <memory>
#include <thread>
#include <vector>

#include "AgentModel/Model.hpp"
#include "AgentModel/ModelStructures.hpp"
#include "AgentModel/UIRenderer.hpp"
#include "AgentModel/SimulationBridge.hpp"

int main() {

	AgentModel::UI::UIRenderer renderer;

	if (!renderer.init(1280, 720, "Brock-Hommes Simulator")) {

		return 1;

	}

	std::unique_ptr<AgentModel::Engine::MarketWorld> market = nullptr;
	std::unique_ptr<AgentModel::Engine::AgentPopulation> population = nullptr;

	const auto bridge = std::make_unique<AgentModel::Engine::SimulationBridge>();

	std::thread engine_thread;

	AgentModel::Engine::MarketWorld* raw_market_ptr = nullptr;


	renderer.on_start_engine = [&](uint32_t agent_count) {
		// model initialisation
		market = std::make_unique<AgentModel::Engine::MarketWorld>();
		population = std::make_unique<AgentModel::Engine::AgentPopulation>(agent_count);
		market->x_t_minus_1 = 1.0;
		market->x_t = 1.0;

		raw_market_ptr = market.get();
		engine_thread = std::thread(AgentModel::Engine::run, std::ref(*market), std::ref(*population), std::ref(*bridge));

	};

	renderer.run_loop(*bridge, raw_market_ptr);

	bridge->is_running.store(false, std::memory_order_release);

	if (engine_thread.joinable()) {

		engine_thread.join();

	}

	renderer.shutdown();
	return 0;

}
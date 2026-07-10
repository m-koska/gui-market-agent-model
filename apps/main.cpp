#include <memory>
#include <thread>
#include <vector>

#include "Model.hpp"
#include "ModelStructures.hpp"
#include "UiRenderer.hpp"
#include "SimulationBridge.hpp"

int main() {

	Ui::UiRenderer renderer;

	if (!renderer.init(1280, 720, "Brock-Hommes Simulator")) {

		return 1;

	}

	std::unique_ptr<Model::MarketWorld> market = nullptr;
	std::unique_ptr<Model::AgentPopulation> population = nullptr;

	const auto bridge = std::make_unique<Utils::SimulationBridge>();

	std::thread engine_thread;

	Model::MarketWorld* raw_market_ptr = nullptr;


	renderer.on_start_engine = [&](uint32_t agent_count) {
		// model initialisation
		market = std::make_unique<Model::MarketWorld>();
		population = std::make_unique<Model::AgentPopulation>(agent_count);
		market->x_t_minus_1 = 1.0;
		market->x_t = 1.0;

		raw_market_ptr = market.get();
		engine_thread = std::thread(Model::run, std::ref(*market), std::ref(*population), std::ref(*bridge));

	};

	renderer.run_loop(*bridge, raw_market_ptr);

	bridge->is_running.store(false, std::memory_order_release);

	if (engine_thread.joinable()) {

		engine_thread.join();

	}

	renderer.shutdown();
	return 0;

}
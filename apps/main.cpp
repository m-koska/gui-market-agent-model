#include "AgentModel/UIRenderer.hpp"

int main() {

	AgentModel::UI::UIRenderer renderer;

	if (!renderer.init(1280, 720, "Brock-Hommes Simulator")) {

		return 1;

	}

	AgentModel::Engine::SimulationEngine engine;

	renderer.run_loop(engine);

	engine.simulation_stop();
	renderer.shutdown();

	return 0;

}
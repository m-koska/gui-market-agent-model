#pragma once

#include "ModelStructures.hpp"
#include "SimulationBridge.hpp"

namespace Model {

	void run(MarketWorld &market, AgentPopulation &pop, Utils::SimulationBridge& simulation_bridge);

}

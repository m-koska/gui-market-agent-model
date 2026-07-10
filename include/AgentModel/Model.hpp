#pragma once

#include "ModelStructures.hpp"
#include "SimulationBridge.hpp"

namespace AgentModel::Engine {

	void run(MarketWorld &market, AgentPopulation &pop, SimulationBridge& simulation_bridge);

}

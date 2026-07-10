#pragma once
#include "AgentModel/RingBuffer.hpp"

namespace AgentModel::Engine {

	struct TickData {

		uint32_t step;
		double x_t;

	};

	struct SimulationBridge {

		Utils::RingBuffer<TickData, 16384> buffer{};
		std::atomic<bool> is_running{true};

	};


	struct CandleData {

		double step;  // Pozycja na osi X (np. indeks świecy)
		double open;  // Cena otwarcia
		double high;  // Cena najwyższa
		double low;   // Cena najniższa
		double close; // Cena zamknięcia

	};

}

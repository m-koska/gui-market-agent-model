#pragma once

#include <barrier>
#include <cstdint>
#include <functional>
#include <random>

#include "ModelStructures.hpp"
#include "RingBuffer.hpp"

namespace AgentModel::Engine {

  struct TickData {

    uint32_t step;
    double x_t;

  };

  struct SimulationBridge {

    Utils::RingBuffer<TickData, 16384> buffer{};
    std::atomic<bool> is_running{false};

  };


  struct CandleData {

    double step;  // Pozycja na osi X (np. indeks świecy)
    double open;  // Cena otwarcia
    double high;  // Cena najwyższa
    double low;   // Cena najniższa
    double close; // Cena zamknięcia

  };

  class SimulationEngine {

  private:
    std::unique_ptr<MarketWorld> market = nullptr;
    std::unique_ptr<AgentPopulation> population = nullptr;
    std::unique_ptr<SimulationBridge> bridge = std::make_unique<SimulationBridge>();

    std::vector<std::thread> threads;
    std::unique_ptr<std::barrier<std::function<void()>>> sync_point = nullptr; // created ptr to allow default constructor

    std::random_device rd;
    std::mt19937 market_noise_rng;

    // initial values
    double prob_fundamentalist = 0.5;
    uint32_t current_step = 0;

    int agent_count{};

    // simulation functions
    void on_turn_complete() noexcept; // called after each turn after the worker threads arrive to a certain point
    void reassign_strategies (size_t chunk_begin, size_t chunk_end) const; // chunks of agents to calculate

  public:

    SimulationEngine () : market_noise_rng(rd()) {}
    ~SimulationEngine() {
      simulation_stop();
    }

    void simulation_start(int set_agent_count);
    void simulation_stop();

    // we return a pointer because we cannot initialise the model before
    // the user does not specify agent_count
    [[nodiscard]] MarketWorld* get_market() const { return market.get(); }
    [[nodiscard]] SimulationBridge& get_bridge() const { return *bridge; }

  };

}

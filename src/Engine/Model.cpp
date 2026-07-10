#include "AgentModel/Model.hpp"

#include <barrier>
#include <thread>
#include <iostream>
#include <random>
#include <chrono>

namespace Model {

	void run(MarketWorld &market, AgentPopulation &pop, Utils::SimulationBridge& simulation_bridge) {

		const auto agent_count = pop.strategies.size();

		const uint32_t num_threads = 4;
		const uint32_t chunk_size = agent_count / num_threads;

		double prob_fundamentalist = 0.5;
		uint32_t current_step = 0;

		const auto market_seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
		std::mt19937 market_noise_rng(market_seed);

		// sync point for the simulation, called after each turn
		auto on_turn_complete = [&] () noexcept {

			// 0. Reading the current parameters
			const double degrees_of_freedom = market.noise_degrees_of_freedom.load(std::memory_order_relaxed);
			const double scale = market.noise_scale.load(std::memory_order_relaxed);

			std::student_t_distribution market_noise_dist(std::max(0.1, degrees_of_freedom));
			const double market_shock = market_noise_dist(market_noise_rng) * scale;

			// 1. Setting a new value for price deviation from the fundamental value
			const auto discounter = market.discounter.load(std::memory_order_relaxed);

			const double trend_chaser_percentage =
				static_cast<double>(pop.trend_chaser_count.load(std::memory_order_relaxed)) /
					static_cast<double>(agent_count);

			const auto trend_following_intensity = market.trend_following_intensity.load(std::memory_order_relaxed);

			const auto last_price = market.x_t;
			const auto previous_last_price = market.x_t_minus_1;
			const double new_x = ((trend_chaser_percentage * trend_following_intensity * last_price) / discounter) + market_shock;

			// 2. Calculating the probability of changing the strategy based on past successes

			const double fundamentalist_utility =
					(new_x - (discounter * last_price)) * (-1 * discounter * last_price) -
					market.fundamentalist_cost.load(std::memory_order_relaxed);

			const double trend_chaser_utility =
					(new_x - (discounter * last_price)) *
					((trend_following_intensity * previous_last_price) - (discounter * last_price)) - market.trend_chaser_cost.load(std::memory_order_relaxed);

			const double choice_intensity = market.choice_intensity.load(std::memory_order_relaxed);

			// Stable softmax to avoid double overflow
			const double max_utility = std::max(fundamentalist_utility, trend_chaser_utility);

			const double exp_fund = std::exp(choice_intensity * (fundamentalist_utility - max_utility));
			const double exp_chaser = std::exp(choice_intensity * (trend_chaser_utility - max_utility));

			prob_fundamentalist = exp_fund / (exp_fund + exp_chaser);

			// 3. Updating the market and the agents

			market.x_t_minus_1 = market.x_t;
			market.x_t = new_x;

			pop.fundamentalist_count.store(0, std::memory_order_relaxed);
			pop.trend_chaser_count.store(0, std::memory_order_relaxed);

			Utils::TickData tick{};
			tick.step = current_step++;
			tick.x_t = new_x;

			// 4. Sending data to the UI
			simulation_bridge.buffer.push(tick);

		};

		on_turn_complete();

		std::barrier sync_point(num_threads, on_turn_complete);

		auto reassign_strategies = [&](const size_t begin, const size_t end, const uint32_t thread_index) {

			// separate RNG for each thread to avoid conflicts
			const auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count() + thread_index;
			std::mt19937 rng(seed);
			std::uniform_real_distribution dist(0.0, 1.0);

			// main simulation loop: reassigning strategies
			while (simulation_bridge.is_running.load(std::memory_order_acquire)) {

				uint32_t thread_fundamentalist_count = 0;
				uint32_t thread_trend_chaser_count = 0;

				for (size_t i = begin; i < end; i++) {

					if (dist(rng) < prob_fundamentalist) {

						pop.strategies[i] = Strategy::Fundamentalist;
						++thread_fundamentalist_count;

					} else {

						pop.strategies[i] = Strategy::TrendChaser;
						++thread_trend_chaser_count;

					}

				}

				pop.fundamentalist_count.fetch_add(thread_fundamentalist_count, std::memory_order_relaxed);
				pop.trend_chaser_count.fetch_add(thread_trend_chaser_count, std::memory_order_relaxed);

				// calling on_turn_complete when all threads are done
				sync_point.arrive_and_wait();

			}

		};

		// Starting the threads
		std::vector<std::thread> threads;
		threads.reserve(num_threads);

		for (int i = 0; i < num_threads; ++i) {

			const size_t start = i * chunk_size;
			const size_t end = i == num_threads - 1 ? agent_count :	(i + 1) * chunk_size;

			threads.emplace_back(reassign_strategies, start, end, i);

		}

		for (auto& thread : threads) {

			if (thread.joinable()) {

				thread.join();

			}

		}

	}


}

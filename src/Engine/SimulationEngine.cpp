#include "AgentModel/SimulationEngine.hpp"

namespace AgentModel::Engine {

	void SimulationEngine::on_turn_complete() noexcept {

		// 0. Reading the current parameters
		const double degrees_of_freedom = get_market()->noise_degrees_of_freedom.load(std::memory_order_relaxed);
		const double scale = get_market()->noise_scale.load(std::memory_order_relaxed);

		std::student_t_distribution market_noise_dist(std::max(0.1, degrees_of_freedom));
		const double market_shock = market_noise_dist(market_noise_rng) * scale;

		// 1. Setting a new value for price deviation from the fundamental value
		const auto discounter = get_market()->discounter.load(std::memory_order_relaxed);

		const double trend_chaser_percentage =
			static_cast<double>(population->trend_chaser_count.load(std::memory_order_relaxed)) /
				static_cast<double>(agent_count);

		const auto trend_following_intensity = get_market()->trend_following_intensity.load(std::memory_order_relaxed);

		const auto last_price = get_market()->x_t;
		const auto previous_last_price = get_market()->x_t_minus_1;
		const double new_x = ((trend_chaser_percentage * trend_following_intensity * last_price) / discounter) + market_shock;

		// 2. Calculating the probability of changing the strategy based on past successes

		const double fundamentalist_utility =
			(new_x - (discounter * last_price)) * (-1 * discounter * last_price) -
				get_market()->fundamentalist_cost.load(std::memory_order_relaxed);

		const double trend_chaser_utility =
			(new_x - (discounter * last_price)) *
				((trend_following_intensity * previous_last_price) - (discounter * last_price)) - get_market()->trend_chaser_cost.load(std::memory_order_relaxed);

		const double choice_intensity = get_market()->choice_intensity.load(std::memory_order_relaxed);

		// Stable softmax to avoid double overflow
		const double max_utility = std::max(fundamentalist_utility, trend_chaser_utility);

		const double exp_fund = std::exp(choice_intensity * (fundamentalist_utility - max_utility));
		const double exp_chaser = std::exp(choice_intensity * (trend_chaser_utility - max_utility));

		prob_fundamentalist = exp_fund / (exp_fund + exp_chaser);

		// 3. Updating the market and the agents

		get_market()->x_t_minus_1 = get_market()->x_t;
		get_market()->x_t = new_x;

		population->fundamentalist_count.store(0, std::memory_order_relaxed);
		population->trend_chaser_count.store(0, std::memory_order_relaxed);

		TickData tick{};
		tick.step = current_step++;
		tick.x_t = new_x;

		// 4. Sending data to the UI
		get_bridge().buffer.push(tick);

	}

	void SimulationEngine::reassign_strategies (const size_t chunk_begin, const size_t chunk_end) const {

		std::random_device thread_rd; // replaced with separate random_device
		// I just noticed, that it's not running on the critical path (while loop) so the threads can take its time to
		// initialise a true rng
		std::mt19937 rng(thread_rd());
		std::uniform_real_distribution dist(0.0, 1.0);

		// main simulation loop: reassigning strategies
		while (get_bridge().is_running.load(std::memory_order_acquire)) {

			uint32_t thread_fundamentalist_count = 0;
			uint32_t thread_trend_chaser_count = 0;

			for (size_t i = chunk_begin; i < chunk_end; i++) {

				if (dist(rng) < prob_fundamentalist) {

					population->strategies[i] = Strategy::Fundamentalist;
					++thread_fundamentalist_count;

				} else {

					population->strategies[i] = Strategy::TrendChaser;
					++thread_trend_chaser_count;

				}

			}

			population->fundamentalist_count.fetch_add(thread_fundamentalist_count, std::memory_order_relaxed);
			population->trend_chaser_count.fetch_add(thread_trend_chaser_count, std::memory_order_relaxed);

			// calling on_turn_complete when all threads are done
			sync_point->arrive_and_wait();

		}

	}

	void SimulationEngine::simulation_start(const int set_agent_count) {

		if (bridge->is_running.load(std::memory_order_acquire)) {
			throw std::runtime_error("Runtime error: tried to start multiple simulations!");
		}

		market = std::make_unique<MarketWorld>();
		population = std::make_unique<AgentPopulation>(set_agent_count);

		bridge->is_running.store(true, std::memory_order_release);

		agent_count = set_agent_count;

		// optimised for my machine after profiling with flamecharts (ryzen 7 9700x 32 GB 6000 MHz)
		const uint32_t num_threads = 4;
		const uint32_t chunk_size = agent_count / num_threads;

		sync_point = std::make_unique<std::barrier<CompletionFunction>> (
			num_threads,
			CompletionFunction{this}
		);

		on_turn_complete();

		threads.reserve(num_threads);

		for (int i = 0; i < num_threads; ++i) {

			const size_t start = i * chunk_size;
			const size_t end = i == num_threads - 1 ? agent_count :	(i + 1) * chunk_size;

			threads.emplace_back(&SimulationEngine::reassign_strategies, this, start, end);

		}

	}

	void SimulationEngine::simulation_stop() {
		for (auto& thread : threads) {
			if (thread.joinable()) thread.join();
		}
	}
}

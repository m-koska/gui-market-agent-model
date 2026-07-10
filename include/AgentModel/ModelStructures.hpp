#pragma once

#include <vector>
#include <cstdint>
#include <atomic>

namespace AgentModel::Engine {

	enum class Strategy : uint8_t {

		Fundamentalist = 0,
		TrendChaser = 1

	};

	// AgentPopulation struct for storing strategies for each actor
	// Aligned to 64-byte CPU cache line to avoid false sharing,
	// forcing the struct into separate cache line
	struct AgentPopulation {

		std::vector<Strategy> strategies;
		alignas(64) std::atomic<uint32_t> fundamentalist_count;
		alignas(64) std::atomic<uint32_t> trend_chaser_count;

		explicit AgentPopulation(uint32_t agent_count)
			: strategies(agent_count, Strategy::Fundamentalist),
			  fundamentalist_count(agent_count / 2),
			  trend_chaser_count(agent_count / 2) {}

	};

	// Brock-Hommes model market variables and parameters
	struct alignas(64) MarketWorld {

		// Price history for reevaluating strategies
		double x_t{0.0};
		double x_t_minus_1{0.0};

		// Parameters
		std::atomic<double> discounter{1.1};                // R = 1 + r	|	risk-free rate
		std::atomic<double> choice_intensity{3.5};          // beta		|	propensity to change strategy
		std::atomic<double> trend_following_intensity{1.2}; // g			|	how much the trend affects strategy for trend chasers
		std::atomic<double> fundamentalist_cost{1.0};       // C_F			|	cost for choosing the fundamentalist strategy (market research)
		std::atomic<double> trend_chaser_cost{0.0};         // C_C			|	trend chasing is free, pure vibes based investing

		// market noise parameters for t distribution
		std::atomic<double> noise_degrees_of_freedom{3.0};
		std::atomic<double> noise_scale{0.05};
	};

}
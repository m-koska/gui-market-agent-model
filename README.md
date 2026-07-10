# Stochastic Brock-Hommes Model Simulator

Multithreaded market simulator implementing stochastic version of Brock-Hommes model. The project demonstrates low-latency engineering with lock-free structures and realtime UI rendering by a graphical thread fed by a ring buffer.

![Screenshot](screenshot_simulation.png)

## What is the Brock-Hommes Model?

The Brock-Hommes (1998) model is a cornerstone of behavioral finance and agent-based computational economics. Unlike previous models that traditionally assumed perfectly rational actors and efficient markets, this framework explores a market driven by **heterogeneous beliefs**, where different agents have different ideas on how to make the most profit.

The simulation models a population of agents divided into two primary strategies:
1. **Fundamentalists:** Actors who believe the asset price will revert to its intrinsic, fundamental value.
2. **Trend Chasers:** Actors who extrapolate past price movements, ignoring the underlying fundamentals.

#### Why does it matter?
Brock & Hommes demonstrated how dynamically changing heterogeneous beliefs can lead to chaos.

As agents dynamically switch between these strategies based on recent profitability (utility), the market exhibits complex, non-linear dynamics. Depending on the parameters, the system can display stable equilibriums, cyclical bubbles, or severe, chaotic crashes.

This specific project extends the original deterministic model by introducing stochastic, fat-tailed noise (Student's t-distribution) to simulate unpredictable macroeconomic shocks.

#### Practical limitations & important note

> **⚠️ Important note:** From a purely mathematical and economic standpoint, simulating millions of individual agents for this specific model is a huge overkill. Due to **the Law of Large Numbers**, the macroscopic behavior of the population perfectly converges to the expected value of the Softmax probability distribution, meaning each simulation turn could be written in a one line of code. This project, however, is not about implementing a rather primitive model with no use in asset pricing (without budgets, welth distribution, order books etc.). My main goal is to **demonstrate my concurrent architecture**, which I will use in the future to build a more advanced agent model, where the maths will actually  pose a challenge.
>

## Architecture

The simulation engine has been optimised and profiled to my station (Ryzen 7 9700X, 32GB RAM). Although 4 working threads may seem like an arbitrary ammount, it has proven most efficient, as it on the one hand speed up calculation comparing to the one simulation thread solution, and on the other minimises OS context switching.

```mermaid
graph TD
    subgraph Main Thread [Composition Root]
        M[main.cpp]
    end

    subgraph Simulation Engine [Background Thread Pool]
        E[SimulationEngine]
        WP1((Worker Thread 1))
        WP2((Worker Thread 2))
        WP3((Worker Thread 3))
        WP4((Worker Thread 4))
        
        B{std::barrier}
        
        E -->|Spawns| WP1
        E -->|Spawns| WP2
        E -->|Spawns| WP3
        E -->|Spawns| WP4
        
        WP1 -.->|Sync| B
        WP2 -.->|Sync| B
        WP3 -.->|Sync| B
        WP4 -.->|Sync| B
    end

    subgraph Communication [Lock-Free Bridge]
        RB[[Ring Buffer]]
        AT[std::atomic Flags]
    end

    subgraph UI Thread [Event Loop]
        UI[UiRenderer]
        IMG[Dear ImGui / ImPlot]
    end

    M -->|Owns & Instantiates| E
    M -->|Instantiates| UI
    
    UI -->|Borrows Reference| E
    UI -->|Reads Data| RB
    UI -->|Renders| IMG
    
    B -->|On Turn Complete| RB
    E -->|Reads/Writes| AT
    UI -->|Writes Parameters| AT
```

## Mathematical Framework & Model Specification

The simulation is based on the seminal paper by **William A. Brock and Cars G. Hommes (1998)**: *"Heterogeneous beliefs and routes to chaos in a simple asset pricing model"* (Journal of Economic Dynamics and Control).

This model shows how dynamically changing heterogeneous beliefs and strategies generate chaotic price fluctuations. Each agent has a chance of changing his strategy based on past returns.

Let $x_t$ be the asset price deviation from its fundamental value at time $t$, where the fundamental value is a constant.

### 1. The equilibrium price dynamic
The equilibrium price deviation $x_t$ is determined by the proportion of the **Trend Chasers**, previous price and the interest rate:

$$x_t = \frac{n_{c,t-1} \cdot g \cdot x_{t-1}}{R} + \epsilon_t$$

Where:
* $n_{c,t-1}$ is the percentage of the population adopting the **Trend Chaser** strategy in the previous period.
* $g$ is the **Trend Following Intensity** parameter (`trend_following_intensity`).
* $R$ is the discounting factor ($R = 1 + r$, where $r$ is the risk-free interest rate).
* $\epsilon_t$ is the statistical noise, representing exogenous market shocks:
  $$\epsilon_t \sim \text{Student-t}(\nu) \cdot \sigma$$

*(Note: Fundamentalists believe the price will return to its fundamental value, meaning their forecast for deviation is $f_f(x_{t-1}) = 0$, hence they are not included in the numerator).*

### 2. Changing strategies
Strategies are reevaluated at each step of the simulation by calculating their economic utility, which equals the realised return of the strategy minus its operational costs:

* **Fundamentalist Utility ($U_{f,t}$):**
  $$U_{f,t} = (x_t - R x_{t-1})(-R x_{t-1}) - C_f$$

* **Trend Chaser Utility ($U_{c,t}$):**
  $$U_{c,t} = (x_t - R x_{t-1})(g x_{t-2} - R x_{t-1}) - C_c$$

Where $C_f$ is the cost of fundamental research (`fundamentalist_cost`) and $C_c$ is the cost of trend following (typically set to $0$ as it represents pure momentum-based trading). Note that by decreasing the cost of information $C_f$ the market fluctuations decrease.

### 3. Strategy Selection
Agents decide whether to change their strategies each step. The probability $P_{f,t}$ of an agent choosing the Fundamentalist strategy follows an evolutionary Softmax distribution:

$$P_{f,t} = \frac{e^{\beta U_{f,t}}}{e^{\beta U_{f,t}} + e^{\beta U_{c,t}}}$$

The parameter $\beta$ denotes the **Intensity of Choice** (`choice_intensity`).
* As $\beta \to 0$, agents distribute randomly across strategies regardless of profits (high bounded rationality).
* As $\beta \to \infty$, agents instantly and deterministically adopt the single most profitable strategy, causing highly non-linear bifurcations, severe market crashes, and complex chaotic attractors.


## Build Instructions (Linux / macOS)


```bash
git clone https://github.com/m-koska/gui-market-agent-model.git
cd gui-market-agent-model
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j 14
./build/agent_model
```
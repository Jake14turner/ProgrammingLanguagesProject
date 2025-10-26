#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <map>
#include <vector>
#include <string>

// Forward declarations
class Human;
class AnimalPresence;

// --- Simulation Constants ---
const int GRID_WIDTH = 600;
const int GRID_HEIGHT = 600;
const double SIM_TICK_TIME_SECONDS = 10.0;
const double STOP_SIM_AFTER = 600.0;

// Convert real seconds to simulation ticks
int seconds_to_sim_ticks(double s);

// --- SimulationHumanResult Struct ---
struct SimulationHumanResult {
    int sickness_secondary_cases = 0;
    double sickness_animal_hazard = 0.0;
    double sickness_human_hazard = 0.0;
    double sickness_p_zoonotic = 0.0;
};

class Simulation {
public:
    Simulation();

    void add_agent(void* agent);  
    void update();
    void print_results() const;
    std::map<int, SimulationHumanResult> get_results() const;
    double get_current_real_time() const;

    int time_step;

    std::map<int, Human*> human_agents; 
    std::vector<AnimalPresence*> animal_agents;
};

std::map<int, SimulationHumanResult> trial();
void save_data(const std::vector<std::vector<double>>& data, const std::string& value);

#endif
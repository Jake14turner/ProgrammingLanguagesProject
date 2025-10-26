//simulator
#include "simulator.h"
#include "agents.h"
#include "data.h"
#include "display.h"

#include <iostream>
#include <cmath>
#include <fstream>
#include <thread>
#include <chrono>
#include <filesystem>
namespace fs = std::filesystem;
#include <random>
#include <numeric>

using namespace std;

const bool USE_DISPLAY = true;
const bool SAVE_DATA = true;
const int NUM_TRIALS = 1000;
const long GLOBAL_DESC = time(nullptr);
const string MOTION_MODEL_DESC = "h_noisy_interp";
const string DATASET_DESC = "RD";

int seconds_to_sim_ticks(double s) {
    return static_cast<int>(s / SIM_TICK_TIME_SECONDS);
}


Simulation::Simulation() : time_step(0) {}

void Simulation::add_agent(void* agent) {
    Human* h = dynamic_cast<Human*>((Human*)agent);
    if (h)
        human_agents[h->id] = h;
    else
        animal_agents.push_back((AnimalPresence*)agent);
}

void Simulation::update() {
    for (auto& [id, h] : human_agents) h->move(this);
    for (auto* a : animal_agents) a->move(this);

    for (auto& [id, h] : human_agents) h->update(this);
    for (auto* a : animal_agents) a->update(this);

    time_step++;
}

void Simulation::print_results() const {
    for (const auto& [id, h] : human_agents) {
        cout << "*** HUMAN " << id << " ***\n";
        cout << "Final infection model: " << h->infection_model << "\n";
        
        cout << "Contact network:\n";
        for (const auto& [time, contact] : h->contact_network) {
            cout << "  Time " << time << ": " << contact.__repr__() << "\n";
        }
        
        cout << "Sickness records:\n";
        for (const auto& record : h->sickness_records) {
            cout << "  " << record.__repr__() << "\n";
        }
        
        cout << "*** END HUMAN " << id << " ***\n\n";
    }
}

map<int, SimulationHumanResult> Simulation::get_results() const {
    map<int, SimulationHumanResult> res;

    for (const auto& [id, h] : human_agents) {
        SimulationHumanResult r;

        for (const auto& s : h->sickness_records) {
            r.sickness_secondary_cases += s.secondary_cases;
            r.sickness_animal_hazard = s.start_infection_model->experienced_animal_hazard;
            r.sickness_human_hazard = s.start_infection_model->experienced_human_hazard;
            r.sickness_p_zoonotic = s.p_zoonotic;
        }

        res[id] = r;
    }

    return res;
}

double Simulation::get_current_real_time() const {
    return time_step * SIM_TICK_TIME_SECONDS;
}

map<int, SimulationHumanResult> trial() {
    Simulation sim;
    
    if (RD_HUMANS.empty() || RD_ANIMALS.empty()) {
        init_datasets();  
    }
    
    cout << "RD_ANIMALS size: " << RD_ANIMALS.size() << endl;
    cout << "RD_HUMANS size: " << RD_HUMANS.size() << endl;
    
    Display* display = nullptr;
    if (USE_DISPLAY) {
        display = new Display(&sim, GRID_WIDTH, GRID_HEIGHT);
    }
    
    vector<AnimalPresence*> animals = D3_ANIMALS;
    vector<Human*> humans = D3_HUMANS;
    
    for (auto* a : animals) {
        sim.animal_agents.push_back(a);
    }
    
    for (auto* h : humans) {
        sim.human_agents[h->id] = h;
    }
    
    cout << "Sim animals: " << sim.animal_agents.size() << endl;
    cout << "Sim humans: " << sim.human_agents.size() << endl;
    
    bool running = true;
    while (running) {
        sim.update();
        
        if (USE_DISPLAY && display) {
            running = display->render();
        }
        
        if (sim.time_step > seconds_to_sim_ticks(STOP_SIM_AFTER))
            running = false;
    }
    
    if (USE_DISPLAY && display) {
        display->cleanup();
        delete display;
    }
    
    return sim.get_results();
}

void save_data(const vector<vector<double>>& data, const string& value) {
    namespace fs = std::filesystem;

    string out_dir = "data/" + DATASET_DESC + "/" + MOTION_MODEL_DESC;
    fs::create_directories(out_dir);

    string file_path = out_dir + "/" + to_string(GLOBAL_DESC) + "_" + value + ".txt";
    ofstream out(file_path);

    for (const auto& row : data) {
        for (size_t i = 0; i < row.size(); ++i) {
            out << row[i];
            if (i < row.size() - 1) out << ",";
        }
        out << "\n";
    }

    out.close();
    cout << "Saved " << file_path << endl;
}

#ifdef BUILD_SIM_MAIN
int main() {
    cout << "**ZV-Sim**" << endl;

    vector<map<int, SimulationHumanResult>> all_results;
    for (int i = 0; i < NUM_TRIALS; ++i) {
        all_results.push_back(trial());
        this_thread::sleep_for(chrono::milliseconds(1));
    }

    cout << "Simulation complete: " << all_results.size() << " trials." << endl;
    return 0;
}
#endif
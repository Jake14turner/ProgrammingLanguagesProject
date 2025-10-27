//simulator.cpp
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
#include <algorithm>

using namespace std;

const bool USE_DISPLAY = true;  // Set to false for batch trials
const bool SAVE_DATA = true;
const int NUM_TRIALS = 1000;  // Start with 10 for testing, change to 1000 later
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
    
    // Initialize datasets if needed
    if (RD_HUMANS.empty() || RD_ANIMALS.empty()) {
        init_datasets();  
    }
    
    Display* display = nullptr;
    if (USE_DISPLAY) {
        display = new Display(&sim, GRID_WIDTH, GRID_HEIGHT);
    }
    
    // IMPORTANT: Create COPIES of agents for this trial
    // Otherwise all trials share the same agent instances
    vector<AnimalPresence*> animals;
    vector<Human*> humans;
    
    // Deep copy animals from RD_ANIMALS
    for (auto* orig : RD_ANIMALS) {
        animals.push_back(new AnimalPresence(*orig));
    }
    
    // Deep copy humans from RD_HUMANS  
    for (auto* orig : RD_HUMANS) {
        humans.push_back(new Human(*orig));
    }
    
    // Add to simulation
    for (auto* a : animals) {
        sim.animal_agents.push_back(a);
    }
    
    for (auto* h : humans) {
        sim.human_agents[h->id] = h;
    }
    
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
    
    // Get results BEFORE cleaning up agents
    auto results = sim.get_results();
    
    // Clean up trial-specific agents
    for (auto* a : animals) delete a;
    for (auto* h : humans) delete h;
    
    return results;
}

// Calculate statistics for boxplot
struct BoxplotStats {
    double min, q1, median, q3, max;
    vector<double> outliers;
};

BoxplotStats calculate_boxplot_stats(vector<double> data) {
    sort(data.begin(), data.end());
    BoxplotStats stats;
    
    size_t n = data.size();
    stats.min = data[0];
    stats.max = data[n-1];
    
    // Calculate quartiles
    stats.median = (n % 2 == 0) ? 
        (data[n/2-1] + data[n/2]) / 2.0 : data[n/2];
    
    size_t q1_idx = n / 4;
    size_t q3_idx = 3 * n / 4;
    stats.q1 = data[q1_idx];
    stats.q3 = data[q3_idx];
    
    return stats;
}

void save_data_and_plot(const vector<vector<double>>& data, const string& value) {
    string out_dir = "data/" + DATASET_DESC + "/" + MOTION_MODEL_DESC;
    fs::create_directories(out_dir);

    cout << "Saving data for: " << value << endl;
    cout << "Data dimensions: " << data.size() << " x " << (data.empty() ? 0 : data[0].size()) << endl;

    // Create safe filename (replace spaces and special chars with underscores)
    string safe_name = value;
    for (char& c : safe_name) {
        if (c == ' ' || c == '(' || c == ')' || c == '@') {
            c = '_';
        }
    }

    // Save raw data as CSV
    string csv_path = out_dir + "/" + to_string(GLOBAL_DESC) + "_" + safe_name + ".csv";
    ofstream out(csv_path);
    
    for (const auto& row : data) {
        for (size_t i = 0; i < row.size(); ++i) {
            out << row[i];
            if (i < row.size() - 1) out << ",";
        }
        out << "\n";
    }
    out.close();
    cout << "Saved CSV: " << csv_path << endl;

    // Generate plot using Python script (more reliable than matplotlib-cpp)
    try {
        cout << "Generating plot for: " << value << "..." << flush;
        
        // Create a Python script to generate the plot
        string py_script = out_dir + "/plot_" + to_string(GLOBAL_DESC) + "_" + safe_name + ".py";
        ofstream py_out(py_script);
        
        py_out << "import matplotlib\n";
        py_out << "matplotlib.use('Agg')  # Use non-interactive backend\n";
        py_out << "import matplotlib.pyplot as plt\n";
        py_out << "import numpy as np\n";
        py_out << "import sys\n\n";
        
        py_out << "try:\n";
        py_out << "    # Load data\n";
        py_out << "    data = np.loadtxt('" << csv_path << "', delimiter=',')\n";
        py_out << "    \n";
        py_out << "    # Handle 1D array (single human)\n";
        py_out << "    if data.ndim == 1:\n";
        py_out << "        data = data.reshape(1, -1)\n";
        py_out << "    \n";
        py_out << "    print(f'Data shape: {data.shape}')\n";
        py_out << "    \n";
        py_out << "    # Create boxplot\n";
        py_out << "    plt.figure(figsize=(8, 6))\n";
        py_out << "    plt.boxplot(data.T)\n";
        py_out << "    plt.xlabel('Human Agent ID')\n";
        py_out << "    plt.ylabel('" << value << "')\n";
        py_out << "    plt.title('" << value << " by ID (n=" << NUM_TRIALS << " trials)')\n";
        py_out << "    \n";
        py_out << "    num_humans = data.shape[0]\n";
        py_out << "    plt.xticks(range(1, num_humans + 1), range(num_humans))\n";
        py_out << "    \n";
        string plot_path = out_dir + "/" + to_string(GLOBAL_DESC) + "_" + safe_name + ".png";
        py_out << "    plt.savefig('" << plot_path << "', dpi=300, bbox_inches='tight')\n";
        py_out << "    print('Plot saved: " << plot_path << "')\n";
        py_out << "    \n";
        py_out << "except Exception as e:\n";
        py_out << "    print(f'Error: {e}', file=sys.stderr)\n";
        py_out << "    sys.exit(1)\n";
        
        py_out.close();
        
        // Execute Python script synchronously and wait for completion
        string cmd = "python3 \"" + py_script + "\" 2>&1";  // Capture stderr too
        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) {
            cerr << " FAILED (couldn't execute Python)" << endl;
            return;
        }
        
        // Read output
        char buffer[128];
        string result = "";
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }
        
        int return_code = pclose(pipe);
        
        if (return_code == 0) {
            cout << " DONE ✓" << endl;
        } else {
            cout << " FAILED ✗" << endl;
            if (!result.empty()) {
                cerr << "Python output: " << result << endl;
            }
        }
        
    } catch (const exception& e) {
        cerr << "Error generating plot for " << value << ": " << e.what() << endl;
    }
}

#ifdef BUILD_SIM_MAIN
int main() {
    cout << "**ZV-Sim**" << endl;
    cout << "Running " << NUM_TRIALS << " trials..." << endl;

    vector<map<int, SimulationHumanResult>> all_results;
    
    // Run all trials
    for (int i = 0; i < NUM_TRIALS; ++i) {
        if ((i + 1) % 100 == 0) {
            cout << "Progress: " << (i + 1) << "/" << NUM_TRIALS << endl;
        }
        try {
            all_results.push_back(trial());
        } catch (const exception& e) {
            cerr << "Error in trial " << i << ": " << e.what() << endl;
            return 1;
        }
        this_thread::sleep_for(chrono::milliseconds(1));
    }

    cout << "Simulation complete: " << all_results.size() << " trials." << endl;
    
    // Check if we have results
    if (all_results.empty()) {
        cerr << "No results collected!" << endl;
        return 1;
    }
    
    // Determine number of humans (from first trial)
    int num_humans = 0;
    if (!all_results.empty()) {
        for (const auto& [id, _] : all_results[0]) {
            num_humans = max(num_humans, id + 1);
        }
    }
    
    cout << "Number of humans: " << num_humans << endl;
    
    // Initialize data matrices: [num_humans][NUM_TRIALS]
    vector<vector<double>> secondary_cases(num_humans, vector<double>(NUM_TRIALS, 0.0));
    vector<vector<double>> animal_hazard(num_humans, vector<double>(NUM_TRIALS, 0.0));
    vector<vector<double>> human_hazard(num_humans, vector<double>(NUM_TRIALS, 0.0));
    vector<vector<double>> p_zoonotic(num_humans, vector<double>(NUM_TRIALS, 0.0));
    
    // Aggregate results
    for (int trial_num = 0; trial_num < NUM_TRIALS; ++trial_num) {
        const auto& run = all_results[trial_num];
        
        for (const auto& [id, human_res] : run) {
            secondary_cases[id][trial_num] = human_res.sickness_secondary_cases;
            animal_hazard[id][trial_num] = human_res.sickness_animal_hazard;
            human_hazard[id][trial_num] = human_res.sickness_human_hazard;
            p_zoonotic[id][trial_num] = human_res.sickness_p_zoonotic;
        }
    }
    
    // Save data and generate plots
    if (SAVE_DATA) {
        cout << "\n==================================" << endl;
        cout << "GENERATING CHARTS" << endl;
        cout << "==================================" << endl;
        
        vector<string> plot_paths;
        
        save_data_and_plot(secondary_cases, "Secondary Cases");
        plot_paths.push_back("Secondary_Cases");
        
        save_data_and_plot(animal_hazard, "Animal Hazard @ Sickness");
        plot_paths.push_back("Animal_Hazard___Sickness");
        
        save_data_and_plot(human_hazard, "Human Hazard @ Sickness");
        plot_paths.push_back("Human_Hazard___Sickness");
        
        save_data_and_plot(p_zoonotic, "P(Sickness from Zoonotic Origin)");
        plot_paths.push_back("P_Sickness_from_Zoonotic_Origin_");
        
        cout << "\n==================================" << endl;
        cout << "All charts generated!" << endl;
        cout << "==================================" << endl;
        
        // Create HTML dashboard
        cout << "\nCreating results dashboard..." << endl;
        string out_dir = "data/" + DATASET_DESC + "/" + MOTION_MODEL_DESC;
        string html_path = out_dir + "/results_" + to_string(GLOBAL_DESC) + ".html";
        ofstream html(html_path);
        
        html << "<!DOCTYPE html>\n";
        html << "<html><head><title>ZV-Sim Results</title>\n";
        html << "<style>\n";
        html << "body { font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5; }\n";
        html << "h1 { color: #333; }\n";
        html << ".container { max-width: 1200px; margin: 0 auto; background: white; padding: 20px; }\n";
        html << ".plot { margin: 20px 0; text-align: center; }\n";
        html << "img { max-width: 100%; border: 1px solid #ddd; padding: 10px; background: white; }\n";
        html << ".info { background: #e8f4f8; padding: 15px; border-radius: 5px; margin-bottom: 20px; }\n";
        html << "</style></head><body>\n";
        html << "<div class='container'>\n";
        html << "<h1>ZV-Sim Results</h1>\n";
        html << "<div class='info'>\n";
        html << "<p><strong>Dataset:</strong> " << DATASET_DESC << "</p>\n";
        html << "<p><strong>Motion Model:</strong> " << MOTION_MODEL_DESC << "</p>\n";
        html << "<p><strong>Number of Trials:</strong> " << NUM_TRIALS << "</p>\n";
        html << "<p><strong>Number of Humans:</strong> " << num_humans << "</p>\n";
        html << "<p><strong>Timestamp:</strong> " << GLOBAL_DESC << "</p>\n";
        html << "</div>\n";
        
        for (const auto& plot_name : plot_paths) {
            string img_file = to_string(GLOBAL_DESC) + "_" + plot_name + ".png";
            string display_name = plot_name;
            // Replace underscores with spaces for display
            for (char& c : display_name) {
                if (c == '_') c = ' ';
            }
            html << "<div class='plot'>\n";
            html << "<h2>" << display_name << "</h2>\n";
            html << "<img src='" << img_file << "' alt='" << display_name << "'>\n";
            html << "</div>\n";
        }
        
        html << "</div></body></html>\n";
        html.close();
        
        cout << "\n==================================" << endl;
        cout << "OPENING RESULTS DASHBOARD" << endl;
        cout << "==================================" << endl;
        cout << "HTML: " << html_path << endl;
        
        // Small delay to ensure file is written
        this_thread::sleep_for(chrono::milliseconds(500));
        
        // Open the HTML dashboard in default browser
        string open_cmd = "open \"" + html_path + "\"";
        system(open_cmd.c_str());
        
        cout << "Dashboard opened in browser!" << endl;
    }
    
    cout << "Program completed. Exiting cleanly." << endl;
    
    return 0;
}
#endif
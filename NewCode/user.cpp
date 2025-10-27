#include "user.h"
#include "agents.h"
#include "simulator.h"    
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <random>

using namespace user;


bool user::SIMULATE_SPREAD = false;
const float user::HUMAN_HAZARD_HEALTHY = 0.0f;
const float user::HUMAN_HAZARD_SICK = 0.7f;
const float user::HAZARD_DECAY = 0.99f;


void user::human_motion(Human* human, int current_time) {
    if (human->location_history.empty())
        return;

    int next_time = -1;
    LocationRecord next_location{};
    bool found = false;

    for (const auto& [t, loc] : human->location_history) {
        if (t > current_time) {
            next_time = t;
            next_location = loc;
            found = true;
            break;
        }
    }

    if (!found) return;

    float dx = next_location.x - human->location.x;
    float dy = next_location.y - human->location.y;
    float dt = static_cast<float>(next_time - current_time);
    if (dt <= 0) return;

    int max_noise = 8;
    int noise_x = (rand() % (2 * max_noise + 1)) - max_noise;
    int noise_y = (rand() % (2 * max_noise + 1)) - max_noise;

    human->location.x += dx / dt + static_cast<float>(noise_x);
    human->location.y += dy / dt + static_cast<float>(noise_y);
}


void user::animal_motion(AnimalPresence* animal) {
    return;
}


float user::zoonotic_probability_model(HumanSicknessRecord* sickness_record) {
    float hazard_experienced = sickness_record->start_infection_model->experienced_animal_hazard;
    int secondary_cases = sickness_record->secondary_cases;
    return Probability::bayesian_p_zoonotic(hazard_experienced, secondary_cases);
}


InfectionModel::InfectionModel(float output_hazard, float experienced_animal_hazard, float experienced_human_hazard)
    : output_hazard(output_hazard),
      experienced_animal_hazard(experienced_animal_hazard),
      experienced_human_hazard(experienced_human_hazard)
{}

InfectionModel::InfectionModel(const InfectionModel& other)
    : output_hazard(other.output_hazard),
      experienced_animal_hazard(other.experienced_animal_hazard),
      experienced_human_hazard(other.experienced_human_hazard)
{
}

float InfectionModel::total_experienced_hazard() {
    return experienced_animal_hazard + experienced_human_hazard;
}

std::string InfectionModel::__str__() {
    return "(output_hazard=" + std::to_string(output_hazard)
        + ", exp_animal_hazard=" + std::to_string(experienced_animal_hazard)
        + ", exp_human_hazard=" + std::to_string(experienced_human_hazard) + ")";
}

bool user::infection_probability_model(
    Human* human,
    const std::vector<AnimalPresence*>& animal_contacts,
    const std::vector<Human*>& human_contacts
) {
    switch (human->status) {
        case HumanStatus::HEALTHY:
            human->infection_model->output_hazard = HUMAN_HAZARD_HEALTHY;
            break;
        case HumanStatus::SICK:
            human->infection_model->output_hazard = HUMAN_HAZARD_SICK;
            break;
    }

    human->infection_model->experienced_human_hazard *= HAZARD_DECAY;
    human->infection_model->experienced_animal_hazard *= HAZARD_DECAY;

    for (auto* a : animal_contacts) {
        human->infection_model->experienced_animal_hazard +=
            a->infection_model->output_hazard;
    }

    for (auto* h : human_contacts) {
        human->infection_model->experienced_human_hazard +=
            h->infection_model->output_hazard;
    }

    if (!SIMULATE_SPREAD)
        return false;

    float p_got_sick = 1.0f - std::exp(-human->infection_model->total_experienced_hazard());

    float rand_val = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    bool got_sick = rand_val < p_got_sick;

    return got_sick;
}
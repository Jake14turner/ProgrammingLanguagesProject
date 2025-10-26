//agents
#include "agents.h"
#include "user.h"


#include <sstream>
#include <iomanip>
#include "simulator.h"


float CONTACT_NETWORK_PROXIMITY_THRESHOLD = 20;
int INCUBATION_SIM_TIME = seconds_to_sim_ticks(300);


static std::string HumanStatusToString(HumanStatus s) {
    switch (s) {
        case HumanStatus::HEALTHY: return "HumanStatus::HEALTHY";
        case HumanStatus::SICK:    return "HumanStatus::SICK";
        default:                   return "HumanStatus::UNKNOWN";
    }
}


HumanContactRecord::HumanContactRecord(int other_id, HumanStatus other_status, int start_time, float total_proximity, int end_time)
: other_id(other_id),
  other_status(other_status),
  start_time(start_time),
  total_proximity(total_proximity),
  end_time(end_time)
{
    
}

int HumanContactRecord::duration() const {
    return this->end_time - this->start_time;
}

float HumanContactRecord::average_proximity() const {
    int d = this->duration();
    return this->total_proximity / static_cast<float>(d);
}

std::string HumanContactRecord::__repr__() const {
    std::ostringstream oss;
    oss << "(other_id=" << this->other_id
        << ", other_status=" << HumanStatusToString(this->other_status)
        << ", start=" << this->start_time
        << ", duration=" << this->duration()
        << ", avg_proximity=" << this->average_proximity()
        << ")";
    return oss.str();
}


HumanSicknessRecord::HumanSicknessRecord(int start_time, user::InfectionModel* start_infection_model, float p_zoonotic, int end_time, int secondary_cases)
: start_time(start_time),
  start_infection_model(start_infection_model),
  p_zoonotic(p_zoonotic),
  end_time(end_time),
  secondary_cases(secondary_cases)
{
}

std::string HumanSicknessRecord::__repr__() const {
    std::ostringstream oss;
    float animal_hazard = 0.0f;
    float human_hazard = 0.0f;
    if (this->start_infection_model) {
        animal_hazard = this->start_infection_model->experienced_animal_hazard;
        human_hazard  = this->start_infection_model->experienced_human_hazard;
    }

    oss << "(p_zoonotic=" << this->p_zoonotic
        << ", start=" << this->start_time
        << ", end=" << this->end_time
        << ", start_animal_hazard=" << animal_hazard
        << ", start_human_hazard=" << human_hazard
        << ", secondary_cases=" << this->secondary_cases
        << ")";
    return oss.str();
}

AnimalPresence::AnimalPresence(int id, const std::map<int, LocationRecord>& migration_pattern, float radius, float hazard_rate)
: id(id),
  migration_pattern(migration_pattern),
  location(),
  radius(radius),
  infection_model(nullptr)
{
    if (!this->migration_pattern.empty()) {
        auto it = this->migration_pattern.begin();
        this->location = it->second;
    }

    this->infection_model = new user::InfectionModel(hazard_rate, 0.0f, 0.0f);
}

void AnimalPresence::move(Simulation* sim) {
    if (!sim) return;

    auto it = this->migration_pattern.find(sim->time_step);
    if (it != this->migration_pattern.end()) {
        this->location = it->second;
    } else {
        user::animal_motion(this);
    }
}

void AnimalPresence::update(Simulation* sim) {
}


Human::Human(int id, const std::map<int, LocationRecord>& location_history, const std::map<int, HumanStatus>& reports)
: id(id),
  location_history(location_history),
  self_reports(reports),
  location(),
  status(HumanStatus::HEALTHY),
  prev_status(HumanStatus::HEALTHY),
  contact_network(),
  sickness_records(),
  active_contacts(),
  infection_model(nullptr)
{
    if (!this->location_history.empty()) {
        auto it = this->location_history.begin();
        this->location = it->second;
    } else {
        this->location.x = 0.0f;
        this->location.y = 0.0f;
    }

    this->infection_model = new user::InfectionModel(0.0f, 0.0f, 0.0f);
}

void Human::move(Simulation* sim) {
    if (!sim) return;

    auto it = this->location_history.find(sim->time_step);
    if (it != this->location_history.end()) {
        this->location = it->second;
    } else {
        user::human_motion(this, sim->time_step);
    }

    auto rit = this->self_reports.find(sim->time_step);
    if (rit != this->self_reports.end()) {
        this->status = rit->second;
    }
}

void Human::update(Simulation* sim) {
    if (!sim) return;

    std::vector<AnimalPresence*> current_animal_contacts;
    current_animal_contacts.reserve(sim->animal_agents.size());
    for (AnimalPresence* animal : sim->animal_agents) {
        if (!animal) continue;
        float dx = this->location.x - animal->location.x;
        float dy = this->location.y - animal->location.y;
        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist <= animal->radius) {
            current_animal_contacts.push_back(animal);
        }
    }

    for (const auto& kv : sim->human_agents) {
        Human* human = kv.second;
        if (!human) continue;
        if (human->id == this->id) continue;

        float dx = this->location.x - human->location.x;
        float dy = this->location.y - human->location.y;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist <= CONTACT_NETWORK_PROXIMITY_THRESHOLD) {
            auto act_it = this->active_contacts.find(human->id);
            if (act_it != this->active_contacts.end()) {
                act_it->second.total_proximity += dist;
            } else {
                HumanContactRecord record(human->id, human->status, sim->time_step, dist);
                this->active_contacts[human->id] = record;
            }
        } else {
            auto act_it = this->active_contacts.find(human->id);
            if (act_it != this->active_contacts.end()) {
                HumanContactRecord record = act_it->second;
                this->active_contacts.erase(act_it);
                record.end_time = sim->time_step;
                this->contact_network[record.start_time] = record;
            }
        }
    }

    std::vector<Human*> current_human_contacts;
    current_human_contacts.reserve(this->active_contacts.size());
    for (const auto& kv : this->active_contacts) {
        int hid = kv.first;
        auto hit = sim->human_agents.find(hid);
        if (hit != sim->human_agents.end()) {
            current_human_contacts.push_back(hit->second);
        }
    }

    bool got_sick = user::infection_probability_model(this, current_animal_contacts, current_human_contacts);

    if (got_sick && this->status != HumanStatus::SICK) {
        this->status = HumanStatus::SICK;
    }

    if (this->status == HumanStatus::SICK) {
        if (this->prev_status == HumanStatus::HEALTHY) {
            user::InfectionModel* copied_model = nullptr;
            if (this->infection_model) {
                copied_model = new user::InfectionModel(*this->infection_model);
            } else {
                copied_model = new user::InfectionModel(0.0f, 0.0f, 0.0f);
            }

            HumanSicknessRecord record(sim->time_step, copied_model);
            this->sickness_records.push_back(record);
        }

        int sec_cases = this->secondary_cases(sim);
        if (!this->sickness_records.empty()) {
            this->sickness_records.back().secondary_cases = sec_cases;
            this->sickness_records.back().p_zoonotic = user::zoonotic_probability_model(&this->sickness_records.back());
        }
    } else if (this->status == HumanStatus::HEALTHY && this->prev_status == HumanStatus::SICK) {
        if (!this->sickness_records.empty()) {
            this->sickness_records.back().end_time = sim->time_step;
        }
    }

    this->prev_status = this->status;
}

int Human::secondary_cases(Simulation* sim) {
    if (this->sickness_records.empty() || this->status != HumanStatus::SICK) {
        throw std::invalid_argument("Tried to calculate secondary cases when not sick!");
    }

    int infectious_at = this->sickness_records.back().start_time - INCUBATION_SIM_TIME;
    int secondary_cases_count = 0;

    for (const auto& kv : this->contact_network) {
        const HumanContactRecord& c = kv.second;
        if (c.start_time >= infectious_at) {
            auto other_it = sim->human_agents.find(c.other_id);
            if (other_it == sim->human_agents.end()) continue;
            Human* other = other_it->second;
            bool incremented = false;

            for (const HumanSicknessRecord& sickness : other->sickness_records) {
                if (sickness.start_time >= infectious_at && c.other_status == HumanStatus::HEALTHY) {
                    secondary_cases_count += 1;
                    incremented = true;
                    break;
                }
            }

            if (incremented) {
                break;
            }
        }
    }

    return secondary_cases_count;
}
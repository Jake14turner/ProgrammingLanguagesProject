#ifndef AGENTS_H
#define AGENTS_H

#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <memory>
#include "user.h"
#include "simulator.h"

namespace user {
    class InfectionModel;
}

enum class HumanStatus {
    HEALTHY = 0,
    SICK = 1
};

class LocationRecord {
public:
    float x;
    float y;
};

class HumanContactRecord {
public:
    int other_id;
    HumanStatus other_status;  
    int start_time;
    float total_proximity;
    int end_time;

    HumanContactRecord() = default;
    HumanContactRecord(int other_id, HumanStatus other_status, int start_time, float total_proximity, int end_time = -1);

    int duration() const;
    float average_proximity() const;
    std::string __repr__() const;
};

class HumanSicknessRecord {
public:
    int start_time;
    user::InfectionModel* start_infection_model;
    float p_zoonotic;
    int end_time;
    int secondary_cases;
    HumanSicknessRecord() = default;
    HumanSicknessRecord(int start_time, user::InfectionModel* start_infection_model, float p_zoonotic = 0, int end_time = -1, int secondary_cases = 0);

    std::string __repr__() const;
};

class AnimalPresence {
public:
    int id;
    std::map<int, LocationRecord> migration_pattern;
    LocationRecord location;
    float radius;
    user::InfectionModel* infection_model;

    AnimalPresence(int id, const std::map<int, LocationRecord>& migration_pattern, float radius, float hazard_rate);

    void move(Simulation* sim);
    void update(Simulation* sim);
};

class Human {
public:
    int id;
    std::map<int, LocationRecord> location_history;   
    std::map<int, HumanStatus> self_reports;          

    LocationRecord location;
    HumanStatus status;
    HumanStatus prev_status;

    std::map<int, HumanContactRecord> contact_network;  
    std::vector<HumanSicknessRecord> sickness_records;
    std::map<int, HumanContactRecord> active_contacts;  

    user::InfectionModel* infection_model;

    Human(int id, const std::map<int, LocationRecord>& location_history, const std::map<int, HumanStatus>& reports);

    void move(Simulation* sim);
    void update(Simulation* sim);
    int secondary_cases(Simulation* sim);
};

#endif 
//data
#include "data.h"
#include "simulator.h" 
#include <tuple>
#include <map>
#include <vector>

std::map<int, LocationRecord> convert_locations(const std::vector<std::tuple<int, float, float>>& input) {
    std::map<int, LocationRecord> path;
    for (const auto& p : input) {
        int time = seconds_to_sim_ticks(std::get<0>(p));
        float x = std::get<1>(p);
        float y = std::get<2>(p);
        path[time] = LocationRecord{x, y};
    }
    return path;
}

std::map<int, HumanStatus> convert_reports(const std::vector<std::pair<int, HumanStatus>>& input) {
    std::map<int, HumanStatus> reports;
    for (const auto& r : input) {
        int time = seconds_to_sim_ticks(r.first);
        reports[time] = r.second;
    }
    return reports;
}


Human* build_human(int id,
                   const std::vector<std::tuple<int, float, float>>& locations,
                   const std::vector<std::pair<int, HumanStatus>>& reports) {
    auto locs = convert_locations(locations);
    auto reps = convert_reports(reports);
    return new Human(id, locs, reps);
}

AnimalPresence* build_animal(int id,
                             const std::vector<std::tuple<int, float, float>>& locations,
                             float radius,
                             float hazard_rate) {
    auto locs = convert_locations(locations);
    return new AnimalPresence(id, locs, radius, hazard_rate);
}


std::vector<Human*> RD_HUMANS;
std::vector<AnimalPresence*> RD_ANIMALS;

std::vector<Human*> D0_HUMANS;
std::vector<AnimalPresence*> D0_ANIMALS;

std::vector<Human*> D3_HUMANS;
std::vector<AnimalPresence*> D3_ANIMALS;

std::vector<Human*> D4_HUMANS;
std::vector<AnimalPresence*> D4_ANIMALS;


__attribute__((constructor)) void init_datasets() {
    // RD Dataset
    auto RD_H0 = build_human(0, {
        {0, 50, 150}, {30, 200, 150}, {290, 200, 150},
        {300, 300, 150}, {400, 300, 200}, {500, 300, 275}
    }, {{310, HumanStatus::SICK}});
    
    auto RD_H1 = build_human(1, {
        {0, 50, 350}, {260, 200, 350}, {300, 300, 350}, 
        {400, 300, 275}, {500, 300, 200}
    }, {{450, HumanStatus::SICK}});
    
    RD_HUMANS = {RD_H0, RD_H1};
    
    auto RD_A0 = build_animal(0, {{0, 200, 150}}, 40, 0.2);
    auto RD_A1 = build_animal(1, {{0, 200, 350}}, 40, 0.05);
    RD_ANIMALS = {RD_A0, RD_A1};

    // D0 Dataset
    auto D0_H0 = build_human(0,
        {{0, 100, 100}, {200, 500, 100}, {400, 500, 500}, {600, 100, 500}},
        {{380, HumanStatus::SICK}});
    auto D0_H1 = build_human(1,
        {{0, 200, 100}, {200, 160, 100}, {400, 490, 500}, {600, 300, 500}},
        {{500, HumanStatus::SICK}});
    D0_HUMANS = {D0_H0, D0_H1};
    D0_ANIMALS = {build_animal(0, {{0, 450, 150}}, 100, 0.05)};

    // D3 Dataset - FULL VERSION ONLY (remove the short version)
    auto D3_H0 = build_human(0, {
        {0,100,100}, {50,175,175}, {100,250,250}, {150,325,325}, 
        {200,400,400}, {250,475,475}, {300,500,500}
    }, {});
    
    auto D3_H1 = build_human(1, {
        {0,500,100}, {50,425,175}, {100,350,250}, {150,275,325}, 
        {200,200,400}, {250,125,475}, {300,100,500}
    }, {});
    
    auto D3_H2 = build_human(2, {
        {0,100,500}, {50,175,425}, {100,250,350}, {150,325,275}, 
        {200,400,200}, {250,475,125}, {300,500,100}
    }, {});
    
    auto D3_H3 = build_human(3, {
        {0,300,300}, {50,300,300}, {100,300,300}, {150,300,300}, 
        {200,300,300}, {250,300,300}, {300,300,300}  
    }, {});
    
    auto D3_H4 = build_human(4, {
        {200,0,0}, {300,150,150}, {400,300,300}, {500,450,450}, 
        {600,600,600}, {700,600,600}, {800,600,600}
    }, {});
    
    auto D3_H5 = build_human(5, {
        {0,600,0}, {200,480,120}, {400,360,240}, 
        {600,240,360}, {800,120,480}, {1000,0,600}
    }, {});
    
    D3_HUMANS = {D3_H0, D3_H1, D3_H2, D3_H3, D3_H4, D3_H5};
    
    auto D3_A0 = build_animal(0, {{0,450,150}, {50,450,150}, {100,450,150}}, 100, 0.05);
    auto D3_A1 = build_animal(1, {
        {100,200,200}, {150,225,225}, {200,250,250}, 
        {250,275,275}, {300,300,300}, {350,325,325}
    }, 80, 0.3);
    auto D3_A2 = build_animal(2, {{0,100,100}, {100,102,102}, {200,104,104}}, 50, 0.05);
    auto D3_A3 = build_animal(3, {{0,500,500}, {200,400,400}, {400,300,300}}, 60, 0.0);
    
    D3_ANIMALS = {D3_A0, D3_A1, D3_A2, D3_A3};

    // D4 Dataset
    auto D4_H0 = build_human(0, {
        {0,20,20},{200,200,100},{210,220,100},{400,400,100},{600,500,100}
    }, {{550, HumanStatus::SICK}});
    
    auto D4_H1 = build_human(1, {
        {0,20,200},{200,200,300},{210,220,300},{400,400,300},{600,500,300}
    }, {{500, HumanStatus::SICK}});
    
    D4_HUMANS = {D4_H0, D4_H1};
    D4_ANIMALS = {
        build_animal(0, {{0,300,100}}, 45, 0.1), 
        build_animal(1, {{0,200,100}}, 5, 0.005)
    };
}
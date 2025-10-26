#ifndef DATA_H
#define DATA_H

#include "agents.h"
#include <map>
#include <vector>


std::map<int, LocationRecord> convert_locations(const std::vector<std::tuple<int, float, float>>& input);
std::map<int, HumanStatus> convert_reports(const std::vector<std::pair<int, HumanStatus>>& input);

Human* build_human(int id,
                   const std::vector<std::tuple<int, float, float>>& locations,
                   const std::vector<std::pair<int, HumanStatus>>& reports);

AnimalPresence* build_animal(int id,
                             const std::vector<std::tuple<int, float, float>>& locations,
                             float radius,
                             float hazard_rate);


void init_datasets();

extern std::vector<Human*> RD_HUMANS;
extern std::vector<AnimalPresence*> RD_ANIMALS;

extern std::vector<Human*> D0_HUMANS;
extern std::vector<AnimalPresence*> D0_ANIMALS;

extern std::vector<Human*> D3_HUMANS;
extern std::vector<AnimalPresence*> D3_ANIMALS;

extern std::vector<Human*> D4_HUMANS;
extern std::vector<AnimalPresence*> D4_ANIMALS;

#endif
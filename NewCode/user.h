#ifndef USER_H
#define USER_H

#include <map>
#include <vector>
#include <cmath>
#include <string>
#include <random>
#include "agents.h"
#include "probability.h"

class AnimalPresence;
class Human;
class HumanSicknessRecord;

namespace user {
extern const float HAZARD_DECAY;
extern const float HUMAN_HAZARD_HEALTHY;
extern const float HUMAN_HAZARD_SICK;

void human_motion(Human* human, int current_time);
void animal_motion(AnimalPresence* animal);
float zoonotic_probability_model(HumanSicknessRecord* sickness_record);

class InfectionModel {
public:
    float output_hazard;
    float experienced_animal_hazard;
    float experienced_human_hazard;

    InfectionModel(float output_hazard, float experienced_animal_hazard, float experienced_human_hazard);

    float total_experienced_hazard();
    std::string __str__();
};

extern bool SIMULATE_SPREAD;
extern const float HUMAN_HAZARD_HEALTHY;
extern const float HUMAN_HAZARD_SICK;
extern const float HA;



bool infection_probability_model(
    Human* human,
    const std::vector<AnimalPresence*>& animal_contacts, 
    const std::vector<Human*>& human_contacts   
);

} 

#endif 
from itertools import chain
from dataclasses import dataclass
from collections import defaultdict
import copy
import random
import time
import os  # ✅ added
import matplotlib.pyplot as plt
import numpy as np
import tqdm  # ✅ you were using tqdm but hadn’t imported it

from data import *
from agents import *
from display import *


GRID_WIDTH = 600
GRID_HEIGHT = 600

SIM_TICK_TIME_SECONDS = 10
REAL_SECONDS_PER_SIM_SECOND = FRAMES_PER_SECOND * SIM_TICK_TIME_SECONDS

STOP_SIM_AFTER = 600


def seconds_to_sim_ticks(s: float) -> int:
    return int(s / SIM_TICK_TIME_SECONDS)


@dataclass
class SimulationHumanResult:
    sickness_secondary_cases: int = 0
    sickness_animal_hazard: float = 0.0
    sickness_human_hazard: float = 0.0
    sickness_p_zoonotic: float = 0.0


class Simulation:
    def __init__(self):
        self.human_agents: Dict[Human] = {}  # id -> Human
        self.animal_agents: List[AnimalPresence] = []
        self.time_step = 0

    def add_agent(self, agent):
        if isinstance(agent, Human):
            self.human_agents[agent.id] = agent
        else:
            self.animal_agents.append(agent)

    def update(self):
        for agent in chain(self.human_agents.values(), self.animal_agents):
            agent.move(self)

        for agent in chain(self.human_agents.values(), self.animal_agents):
            agent.update(self)

        self.time_step += 1

    def print_results(self):
        for h in self.human_agents.values():
            print(f"*** HUMAN {h.id} ***")
            print(f"Final infection model: {h.infection_model}\n")
            print(f"Contact network: {h.contact_network}\n")
            print(f"Sickness records: {h.sickness_records}")
            print(f"*** END HUMAN {h.id} ***\n")

    def get_results(self):
        res = {}
        for h in self.human_agents.values():
            r = SimulationHumanResult()
            for s in h.sickness_records:
                r.sickness_secondary_cases += s.secondary_cases
                r.sickness_animal_hazard = (
                    s.start_infection_model.experienced_animal_hazard
                )
                r.sickness_human_hazard = (
                    s.start_infection_model.experienced_human_hazard
                )
                r.sickness_p_zoonotic = s.p_zoonotic

                res[h.id] = r

        return res

    def get_current_real_time(self):
        return self.time_step * SIM_TICK_TIME_SECONDS


USE_DISPLAY = True
SAVE_DATA = True
NUM_TRIALS = 1000
GLOBAL_DESC = int(time.time())
MOTION_MODEL_DESC = "h_noisy_interp"
DATASET_DESC = "RD"


def trial():
    sim = Simulation()

    if USE_DISPLAY:
        display = Display(simulation=sim, width=GRID_WIDTH, height=GRID_HEIGHT)

    animals = copy.deepcopy(RD_ANIMALS)
    humans = copy.deepcopy(RD_HUMANS)

    for a in chain(animals, humans):
        sim.add_agent(a)

    running = True
    while running:
        sim.update()

        if USE_DISPLAY:
            running = display.render()

        if sim.time_step > seconds_to_sim_ticks(STOP_SIM_AFTER):
            running = False

    if USE_DISPLAY:
        display.cleanup()

    # sim.print_results()
    return sim.get_results()


def save_data(data, value):
    # ✅ ensure directories exist
    out_dir = f"data/{DATASET_DESC}/{MOTION_MODEL_DESC}"
    os.makedirs(out_dir, exist_ok=True)

    # save .npy file
    np.save(f"{out_dir}/{GLOBAL_DESC}_{value}.npy", data)

    num_rows = data.shape[0]
    plt.figure()
    plt.boxplot(data.T)
    plt.xticks(range(1, num_rows + 1), range(num_rows))
    plt.xlabel("Human Agent ID")
    plt.ylabel(value)
    plt.title(f"{value} by ID (n={NUM_TRIALS} trials)")

    # save plot
    plt.savefig(
        f"{out_dir}/{GLOBAL_DESC}_{value}",
        dpi=300,
        bbox_inches="tight",
    )


if __name__ == "__main__":
    print("**ZV-Sim**")
    print(f"1 sim second = {REAL_SECONDS_PER_SIM_SECOND} real world seconds")

    all_results = []
    for _ in tqdm.tqdm(range(NUM_TRIALS)):
        res = trial()
        all_results.append(res)
        time.sleep(0.001)

    num_humans = len(all_results[0])

    secondary_cases = np.empty((num_humans, NUM_TRIALS))
    animal_hazard = np.empty((num_humans, NUM_TRIALS))
    human_hazard = np.empty((num_humans, NUM_TRIALS))
    p_zoonotic = np.empty((num_humans, NUM_TRIALS))

    for trial_num, run in enumerate(all_results):
        for id, human_res in run.items():
            secondary_cases[id][trial_num] = human_res.sickness_secondary_cases
            animal_hazard[id][trial_num] = human_res.sickness_animal_hazard
            human_hazard[id][trial_num] = human_res.sickness_human_hazard
            p_zoonotic[id][trial_num] = human_res.sickness_p_zoonotic

    if SAVE_DATA:
        save_data(secondary_cases, "Secondary Cases")
        save_data(animal_hazard, "Animal Hazard @ Sickness")
        save_data(human_hazard, "Human Hazard @ Sickness")
        save_data(p_zoonotic, "P(Sickness from Zoonotic Origin)")
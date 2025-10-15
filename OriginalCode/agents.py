from typing import List, Dict
from dataclasses import dataclass
from enum import Enum
from copy import deepcopy
import math
import tqdm


class HumanStatus(Enum):
    HEALTHY = 0
    SICK = 1


from probability import bayesian_p_zoonotic
from simulator import seconds_to_sim_ticks
import user

CONTACT_NETWORK_PROXIMITY_THRESHOLD = 20
INCUBATION_SIM_TIME = seconds_to_sim_ticks(300)


@dataclass
class LocationRecord:
    x: float
    y: float


@dataclass
class HumanContactRecord:
    other_id: int
    other_status: HumanStatus  # other person's status at time of contact
    start_time: int
    total_proximity: float
    end_time: int = None

    def duration(self):
        return self.end_time - self.start_time

    def average_proximity(self):
        return self.total_proximity / self.duration()

    def __repr__(self):
        return f"(other_id={self.other_id}, other_status={self.other_status}, start={self.start_time}, duration={self.duration()}, avg_proximity={self.average_proximity()})"


@dataclass
class HumanSicknessRecord:
    start_time: int
    start_infection_model: user.InfectionModel
    p_zoonotic: float = 0
    end_time: int = None
    secondary_cases: int = 0

    def __repr__(self):
        return f"(p_zoonotic={self.p_zoonotic}, start={self.start_time}, end={self.end_time}, start_animal_hazard={self.start_infection_model.experienced_animal_hazard}, start_human_hazard={self.start_infection_model.experienced_human_hazard}, secondary_cases={self.secondary_cases})"


@dataclass
class Human:
    def __init__(
        self,
        id: int,
        location_history: Dict[int, LocationRecord],
        reports: Dict[int, HumanStatus],
    ):
        self.id: int = id
        self.location_history: Dict[int, LocationRecord] = (
            location_history  # time -> location
        )
        self.self_reports: Dict[int, HumanStatus] = reports  # time -> report

        self.location: LocationRecord = self.location_history[
            min(self.location_history)
        ]
        self.status: HumanStatus = HumanStatus.HEALTHY
        self.prev_status: HumanStatus = HumanStatus.HEALTHY

        self.contact_network: Dict[int, HumanContactRecord] = {}  # time -> contact
        self.sickness_records: List[HumanSicknessRecord] = []
        self.active_contacts: Dict[int, HumanContactRecord] = {}  # other id -> contact

        self.infection_model: user.InfectionModel = user.InfectionModel(
            output_hazard=0.0,
            experienced_animal_hazard=0.0,
            experienced_human_hazard=0.0,
        )

    def move(self, sim):
        if sim.time_step in self.location_history:
            self.location = self.location_history[sim.time_step]
        else:
            user.human_motion(self, sim.time_step)

        if sim.time_step in self.self_reports:
            self.status = self.self_reports[sim.time_step]

    def update(self, sim):
        # check if previous contacts are sick, update filter

        # check if in animal radius, update filter
        current_animal_contacts: List[AnimalPresence] = []
        for animal in sim.animal_agents:
            dx = self.location.x - animal.location.x
            dy = self.location.y - animal.location.y

            dist = math.sqrt(dx**2 + dy**2)
            if dist <= animal.radius:
                # self.hazard_experienced += animal.hazard_rate
                current_animal_contacts.append(animal)

        # check if in contact with a person, update network + filter
        for human in sim.human_agents.values():
            if human.id == self.id:
                continue

            dx = self.location.x - human.location.x
            dy = self.location.y - human.location.y
            dist = math.sqrt(dx**2 + dy**2)

            if dist <= CONTACT_NETWORK_PROXIMITY_THRESHOLD:
                if human.id in self.active_contacts:
                    # we were already in contact with this person
                    self.active_contacts[human.id].total_proximity += dist
                else:
                    # otherwise create a new contact record
                    record = HumanContactRecord(
                        other_id=human.id,
                        other_status=human.status,
                        start_time=sim.time_step,
                        total_proximity=dist,
                    )
                    self.active_contacts[human.id] = record
            else:
                # check if we went out of conact
                if human.id in self.active_contacts:
                    record = self.active_contacts.pop(human.id)
                    record.end_time = sim.time_step
                    self.contact_network[record.start_time] = record

        current_human_contacts = [
            sim.human_agents[h] for h in self.active_contacts.keys()
        ]

        got_sick = user.infection_probability_model(
            self, current_animal_contacts, current_human_contacts
        )
        # add sickness event / update status if simulated sick
        if got_sick and self.status != HumanStatus.SICK:
            # print(f"t={sim.time_step} Human {self.id} simulated sick!")
            self.status = HumanStatus.SICK

        # calculate probabilities
        if self.status == HumanStatus.SICK:

            if self.prev_status == HumanStatus.HEALTHY:
                record = HumanSicknessRecord(
                    start_time=sim.time_step,
                    start_infection_model=deepcopy(self.infection_model),
                )
                self.sickness_records.append(record)

            secondary_cases = self.secondary_cases(sim)
            self.sickness_records[-1].secondary_cases = secondary_cases

            self.sickness_records[-1].p_zoonotic = user.zoonotic_probability_model(
                self.sickness_records[-1]
            )

        elif (
            self.status == HumanStatus.HEALTHY and self.prev_status == HumanStatus.SICK
        ):
            self.sickness_records[-1].end_time = sim.time_step

        self.prev_status = self.status

    # only counts one case per sick contacted individual
    def secondary_cases(self, sim):
        if len(self.sickness_records) == 0 or self.status != HumanStatus.SICK:
            raise ValueError("Tried to calculate secondary cases when not sick!")

        infectious_at = self.sickness_records[-1].start_time - INCUBATION_SIM_TIME
        secondary_cases = 0

        for c in self.contact_network.values():
            if c.start_time >= infectious_at:
                other = sim.human_agents[c.other_id]
                incremented = False
                for sickness in other.sickness_records:
                    if (
                        sickness.start_time >= infectious_at
                        and c.other_status == HumanStatus.HEALTHY
                    ):
                        secondary_cases += 1
                        incremented = True
                        break

                if incremented:
                    # only count each contacted person once
                    break

        return secondary_cases


class AnimalPresence:
    location: LocationRecord
    radius: float

    def __init__(
        self, id: int, migration_pattern: Dict[int, LocationRecord], radius, hazard_rate
    ):
        self.id: int = id
        self.migration_pattern: Dict[int, LocationRecord] = migration_pattern
        self.location: LocationRecord = self.migration_pattern[
            min(self.migration_pattern)
        ]
        self.radius: float = radius
        self.infection_model: user.InfectionModel = user.InfectionModel(
            output_hazard=hazard_rate,
            experienced_animal_hazard=0.0,
            experienced_human_hazard=0.0,
        )

    def move(self, sim):
        if sim.time_step in self.migration_pattern:
            self.location = self.migration_pattern[sim.time_step]
        else:
            user.animal_motion(self)

    def update(self, sim):
        pass

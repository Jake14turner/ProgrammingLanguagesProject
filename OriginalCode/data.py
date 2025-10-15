from agents import *
from simulator import seconds_to_sim_ticks


def convert_locations(input):
    path = {}
    for p in input:
        path[seconds_to_sim_ticks(p[0])] = LocationRecord(x=p[1], y=p[2])
    return path


def convert_reports(input):
    reports = {}
    for r in input:
        reports[seconds_to_sim_ticks(r[0])] = r[1]
    return reports


def build_human(id, locations, reports):
    return Human(
        id=id,
        location_history=convert_locations(locations),
        reports=convert_reports(reports),
    )


def build_animal(id, locations, radius, hazard_rate):
    return AnimalPresence(
        id=id,
        migration_pattern=convert_locations(locations),
        radius=radius,
        hazard_rate=hazard_rate,
    )


### REPORT DATASET (RD) ###
# Two human agents, H0 spends much longer in presence of high hazard A0.
# H1 spends much less time in presence of low hazard A1, and additionally only gets sick after contacting H0
# H1's location data is incomplete, and doesn't directly include them contacting A1

RD_H0_LOCATIONS = [
    (0, 50, 150),
    (30, 200, 150),
    (290, 200, 150),
    (300, 300, 150),
    (400, 300, 200),
    (500, 300, 275),
]
RD_H0_REPORTS = [(310, HumanStatus.SICK)]
RD_H0 = build_human(0, RD_H0_LOCATIONS, RD_H0_REPORTS)

RD_H1_LOCATIONS = [
    (0, 50, 350),
    (260, 200, 350),
    (300, 300, 350),
    (400, 300, 275),
    (500, 300, 200),
]
RD_H1_REPORTS = [(450, HumanStatus.SICK)]
RD_H1 = build_human(1, RD_H1_LOCATIONS, RD_H1_REPORTS)


RD_HUMANS = [RD_H0, RD_H1]

RD_A0_LOCATIONS = [(0, 200, 150)]
RD_A0_RADIUS = 40
RD_A0_HAZARD_RATE = 0.2
RD_A0 = build_animal(0, RD_A0_LOCATIONS, RD_A0_RADIUS, RD_A0_HAZARD_RATE)

RD_A1_LOCATIONS = [(0, 200, 350)]
RD_A1_RADIUS = 40
RD_A1_HAZARD_RATE = 0.05
RD_A1 = build_animal(1, RD_A1_LOCATIONS, RD_A1_RADIUS, RD_A1_HAZARD_RATE)

RD_ANIMALS = [RD_A0, RD_A1]

### EXPERIMENTAL / AD-HOC DATASETS ###

#### DATASET 0 ####
# Humans: 2
# Animal presences: 1
# H0 -> Animal contact, contacts H1 afterward; expect high P(zoonotic)
# H1 -> No animal contact; expect zero P(zoonotic)
D0_H0_LOCATIONS = [
    (0, 100, 100),
    (200, 500, 100),
    (400, 500, 500),
    (600, 100, 500),
]  # contacts animal
D0_H0_REPORTS = [(380, HumanStatus.SICK)]
D0_H0 = build_human(0, D0_H0_LOCATIONS, D0_H0_REPORTS)

D0_H1_LOCATIONS = [
    (0, 200, 100),
    (200, 160, 100),
    (400, 490, 500),
    (600, 300, 500),
]  # contacts other human, does not contact animal
D0_H1_REPORTS = [(500, HumanStatus.SICK)]
D0_H1 = build_human(1, D0_H1_LOCATIONS, D0_H1_REPORTS)

D0_HUMANS = [D0_H0, D0_H1]

D0_A0_LOCATIONS = [(0, 450, 150)]
D0_A0_RADIUS = 100
D0_A0_HAZARD_RATE = 0.05
D0_A0 = build_animal(0, D0_A0_LOCATIONS, D0_A0_RADIUS, D0_A0_HAZARD_RATE)

D0_ANIMALS = [D0_A0]


#### DATASET 3 ####
# Larger scale simulation
D3_H0_LOCATIONS = [
    (0, 100, 100),
    (50, 175, 175),
    (100, 250, 250),
    (150, 325, 325),
    (200, 400, 400),
    (250, 475, 475),
    (300, 500, 500),
]
D3_H0_REPORTS = []
D3_H0 = build_human(0, D3_H0_LOCATIONS, D3_H0_REPORTS)

D3_H1_LOCATIONS = [
    (0, 500, 100),
    (50, 425, 175),
    (100, 350, 250),
    (150, 275, 325),
    (200, 200, 400),
    (250, 125, 475),
    (300, 100, 500),
]
D3_H1_REPORTS = []
D3_H1 = build_human(1, D3_H1_LOCATIONS, D3_H1_REPORTS)

D3_H2_LOCATIONS = [
    (0, 100, 500),
    (50, 175, 425),
    (100, 250, 350),
    (150, 325, 275),
    (200, 400, 200),
    (250, 475, 125),
    (300, 500, 100),
]
D3_H2_REPORTS = []
D3_H2 = build_human(2, D3_H2_LOCATIONS, D3_H2_REPORTS)

D3_H3_LOCATIONS = [(t, 300, 300) for t in range(0, 1001, 50)]
D3_H3_REPORTS = []
D3_H3 = build_human(3, D3_H3_LOCATIONS, D3_H3_REPORTS)

D3_H4_LOCATIONS = [
    (200, 0, 0),
    (300, 150, 150),
    (400, 300, 300),
    (500, 450, 450),
    (600, 600, 600),
    (700, 600, 600),
    (800, 600, 600),
]
D3_H4_REPORTS = []
D3_H4 = build_human(4, D3_H4_LOCATIONS, D3_H4_REPORTS)

D3_H5_LOCATIONS = [
    (0, 600, 0),
    (200, 480, 120),
    (400, 360, 240),
    (600, 240, 360),
    (800, 120, 480),
    (1000, 0, 600),
]
D3_H5_REPORTS = []
D3_H5 = build_human(5, D3_H5_LOCATIONS, D3_H5_REPORTS)

D3_HUMANS = [D3_H0, D3_H1, D3_H2, D3_H3, D3_H4, D3_H5]

D3_A0_LOCATIONS = [(t, 450, 150) for t in range(0, 1001, 50)]
D3_A0_RADIUS = 100
D3_A0_HAZARD_RATE = 0.05
D3_A0 = build_animal(0, D3_A0_LOCATIONS, D3_A0_RADIUS, D3_A0_HAZARD_RATE)

D3_A1_LOCATIONS = [
    (t, int(200 + (200 * (t - 100) / 800)), int(200 + (200 * (t - 100) / 800)))
    for t in range(100, 901, 50)
]
D3_A1_RADIUS = 80
D3_A1_HAZARD_RATE = 0.3
D3_A1 = build_animal(1, D3_A1_LOCATIONS, D3_A1_RADIUS, D3_A1_HAZARD_RATE)

D3_A2_LOCATIONS = [
    (t, int(100 + 20 * (t / 1000)), int(100 + 20 * (t / 1000)))
    for t in range(0, 1001, 50)
]
D3_A2_RADIUS = 50
D3_A2_HAZARD_RATE = 0.05
D3_A2 = build_animal(2, D3_A2_LOCATIONS, D3_A2_RADIUS, D3_A2_HAZARD_RATE)

D3_A3_LOCATIONS = [
    (t, int(500 - 400 * (t / 1000)), int(500 - 400 * (t / 1000)))
    for t in range(0, 1001, 50)
]
D3_A3_RADIUS = 60
D3_A3_HAZARD_RATE = 0.0
D3_A3 = build_animal(3, D3_A3_LOCATIONS, D3_A3_RADIUS, D3_A3_HAZARD_RATE)

D3_ANIMALS = [D3_A0, D3_A1, D3_A2, D3_A3]


#### DATASET 4 ####

D4_H0_LOCATIONS = [
    (0, 20, 20),
    (200, 200, 100),
    (210, 220, 100),
    (400, 400, 100),
    (600, 500, 100),
]
D4_H0_REPORTS = [(550, HumanStatus.SICK)]
D4_H0 = build_human(0, D4_H0_LOCATIONS, D4_H0_REPORTS)

D4_H1_LOCATIONS = [
    (0, 20, 200),
    (200, 200, 300),
    (210, 220, 300),
    (400, 400, 300),
    (600, 500, 300),
]
D4_H1_REPORTS = [(500, HumanStatus.SICK)]
D4_H1 = build_human(1, D4_H1_LOCATIONS, D4_H1_REPORTS)

D4_H2_LOCATIONS = [
    (0, 200, 300),
]
D4_H2_REPORTS = [(0, HumanStatus.SICK)]
D4_H2 = build_human(2, D4_H2_LOCATIONS, D4_H2_REPORTS)

D4_H3_LOCATIONS = [
    (0, 300, 300),
]
D4_H3_REPORTS = [(0, HumanStatus.SICK)]
D4_H3 = build_human(3, D4_H3_LOCATIONS, D4_H3_REPORTS)

D4_HUMANS = [D4_H0, D4_H1, D4_H2, D4_H3]

D4_A0_LOCATIONS = [(0, 300, 100)]
D4_A0_RADIUS = 45
D4_A0_HAZARD_RATE = 0.1
D4_A0 = build_animal(0, D4_A0_LOCATIONS, D4_A0_RADIUS, D4_A0_HAZARD_RATE)

D4_A1_LOCATIONS = [(0, 200, 100)]
D4_A1_RADIUS = 5
D4_A1_HAZARD_RATE = 0.005
D4_A1 = build_animal(1, D4_A1_LOCATIONS, D4_A1_RADIUS, D4_A1_HAZARD_RATE)

D4_ANIMALS = [D4_A0, D4_A1]

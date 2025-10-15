import math

from scipy.stats import poisson

PRIOR_PROBABILITY_ZOONOTIC = 0.01  # TODO: find a data-informed estimate for this
EXPECTED_SECONDARY_CASES_ZOONOTIC = 0.1
EXPECTED_SECONDARY_CASES_NON_ZOONOTIC = 2.0


def p_hazard_given_zoonotic(hazard_experienced):
    return 1 - math.exp(-hazard_experienced)


def p_secondary_cases_given_zoonotic(k):
    return poisson.pmf(k, EXPECTED_SECONDARY_CASES_ZOONOTIC)


def p_secondary_cases_given_non_zoonotic(k):
    return poisson.pmf(k, EXPECTED_SECONDARY_CASES_NON_ZOONOTIC)


def bayesian_p_zoonotic(hazard_experienced, secondary_cases):
    f_E = p_hazard_given_zoonotic(hazard_experienced)
    g_k = p_secondary_cases_given_zoonotic(secondary_cases)
    h_k = p_secondary_cases_given_non_zoonotic(secondary_cases)

    return (f_E * g_k * PRIOR_PROBABILITY_ZOONOTIC) / (
        f_E * g_k * PRIOR_PROBABILITY_ZOONOTIC
        + ((1 - f_E) * h_k) * (1 - PRIOR_PROBABILITY_ZOONOTIC)
    )

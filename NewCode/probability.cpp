//probability
#include "probability.h"
#include <cmath>

namespace Probability {

    const double PRIOR_PROBABILITY_ZOONOTIC = 0.01; 
    const double EXPECTED_SECONDARY_CASES_ZOONOTIC = 0.1;
    const double EXPECTED_SECONDARY_CASES_NON_ZOONOTIC = 2.0;

    static double poisson_pmf(int k, double lambda) {
        return std::exp(-lambda) * std::pow(lambda, k) / std::tgamma(k + 1);
    }

    double p_hazard_given_zoonotic(double hazard_experienced) {
        return 1.0 - std::exp(-hazard_experienced);
    }

    double p_secondary_cases_given_zoonotic(int k) {
        return poisson_pmf(k, EXPECTED_SECONDARY_CASES_ZOONOTIC);
    }

    double p_secondary_cases_given_non_zoonotic(int k) {
        return poisson_pmf(k, EXPECTED_SECONDARY_CASES_NON_ZOONOTIC);
    }

    double bayesian_p_zoonotic(double hazard_experienced, int secondary_cases) {
        double f_E = p_hazard_given_zoonotic(hazard_experienced);
        double g_k = p_secondary_cases_given_zoonotic(secondary_cases);
        double h_k = p_secondary_cases_given_non_zoonotic(secondary_cases);

        double numerator = f_E * g_k * PRIOR_PROBABILITY_ZOONOTIC;
        double denominator = numerator +
                             ((1.0 - f_E) * h_k) * (1.0 - PRIOR_PROBABILITY_ZOONOTIC);

        if (denominator == 0.0) return 0.0;
        return numerator / denominator;
    }

}
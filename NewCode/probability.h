#ifndef PROBABILITY_H
#define PROBABILITY_H

namespace Probability {

    extern const double PRIOR_PROBABILITY_ZOONOTIC;
    extern const double EXPECTED_SECONDARY_CASES_ZOONOTIC;
    extern const double EXPECTED_SECONDARY_CASES_NON_ZOONOTIC;

    double p_hazard_given_zoonotic(double hazard_experienced);
    double p_secondary_cases_given_zoonotic(int k);
    double p_secondary_cases_given_non_zoonotic(int k);
    double bayesian_p_zoonotic(double hazard_experienced, int secondary_cases);

}

#endif 
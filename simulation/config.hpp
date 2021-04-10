#ifndef CONFIG_H
#define CONFIG_H

#define CONFIG_DEFAULT    0
#define CONFIG_VALIDATION 1
#define CONFIG_EVALUATION 2
#define CONFIG_PROFILING  3

#ifndef CONFIG
#define CONFIG CONFIG_DEFAULT
#endif

#if CONFIG == CONFIG_DEFAULT
    #include "simulation/config_default.hpp"
#elif CONFIG == CONFIG_VALIDATION
    #include "simulation/config_validation.hpp"
#elif CONFIG == CONFIG_EVALUATION
    #include "simulation/config_evaluation.hpp"
#elif CONFIG == CONFIG_PROFILING
    #include "simulation/config_profiling.hpp"
#endif

#endif

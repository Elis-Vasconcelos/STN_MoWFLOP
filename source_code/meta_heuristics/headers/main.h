#ifndef MAIN_PROGRAM_H
#define MAIN_PROGRAM_H

#include "../headers/instance_info.h"

#include "../headers/global_modules/generate_initial_population/population.h"
#include "../headers/global_modules/generate_initial_population/generate_rSolution.h"

#include "../headers/global_modules/genetic_operators/mutation.h"
#include "../headers/global_modules/genetic_operators/crossover.h"

#include "../headers/global_modules/isEqual.h"

#include "../headers/metaheuristics/moead/moead.h"
#include "../headers/metaheuristics/moead/modules/generate_neighborhood.h"
#include "../headers/metaheuristics/moead/modules/generate_weight_vectors.h"
#include "../headers/metaheuristics/moead/modules/get_best_z_point.h"
#include "../headers/metaheuristics/moead/modules/tchebycheff.h"

#include "./metaheuristics/nsga2/nsga2.h"
#include "./metaheuristics/nsga2/modules/binary_tournament.h"
#include "./metaheuristics/nsga2/modules/crowding_distance.h"
#include "./metaheuristics/nsga2/modules/non_dominated_sorting.h"
#include "./metaheuristics/nsga2/modules/arena_non_dominated_sorting.h"

#include "../headers/utils/features_csv.h"
#include "../headers/adaptative_grid/BoundedParetoSet.h"

#endif
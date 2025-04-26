#include "../../../../headers/global_modules/generate_initial_population/generate_rSolution.h"
#include "../../../../headers/global_modules/dominates.h"
#include "../../../../headers/global_modules/isEqual.h"
#include "../../../../headers/metaheuristics/nsga2/modules/update_population.h"

#include <vector>

void updatePopulation(vector<Solution*>& population) {
  for (int i = 0; i < population.size(); i++) {
    bool isDominated = false;

    for (int j = 0; j < population.size(); j++) {
      if (i != j && dominates(*population[j], *population[i])) {
        isDominated = true;
        population.erase(population.begin() + i);
        i--;
        break;
      }
    }

    if (!isDominated) {
      vector<int> copies;
      for (int j = 0; j < population.size(); j++) {
        if (i != j && isEqual(*population[j], *population[i])) {
          copies.push_back(j);
        }
      }

      if (!copies.empty()) {
        for (int k = copies.size() - 1; k >= 0; k--) {
          population.erase(population.begin() + copies[k]);
        }
      }
    }
  }
}
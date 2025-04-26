#include <utility>
#include <random>
#include "../../../../headers/global_modules/generate_initial_population/generate_rSolution.h"
#include "../../../../headers/metaheuristics/nsga2/modules/binary_tournament.h"

Solution** binary_tournament(vector<Solution*>& population){
  Solution** parents = new Solution*[2];

  parents[0] = new Solution;
  parents[1] = new Solution;

  int i1 = rand() % population.size();
  int i2 = rand() % population.size();
  int i3 = rand() % population.size();
  int i4 = rand() % population.size();


  Solution * candidate1 = population[i1];
  Solution * candidate2 = population[i2];
  Solution * candidate3 = population[i3];
  Solution * candidate4 = population[i4];


  if(candidate1->fitness.first >= candidate2->fitness.first){
    *parents[0] = *candidate1; 
  } else{
    *parents[0] = *candidate2;
  }  

  if(candidate3->fitness.second >= candidate4->fitness.second) *parents[1] = *candidate3;
  else *parents[1] = *candidate4;

  return parents;
}
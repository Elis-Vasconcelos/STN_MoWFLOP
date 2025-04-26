#include <utility>
#include <vector>
#include <iostream>
#include <ctime> 
#include <algorithm>
#include <random>

#include "../../../headers/global_modules/generate_initial_population/population.h"
#include "../../../headers/global_modules/generate_initial_population/generate_rSolution.h"

#include "../../../headers/metaheuristics/moead/modules/generate_weight_vectors.h"
#include "../../../headers/metaheuristics/moead/modules/generate_neighborhood.h"
#include "../../../headers/metaheuristics/moead/modules/get_best_z_point.h"
#include "../../../headers/metaheuristics/moead/modules/tchebycheff.h"

#include "../../../headers/global_modules/genetic_operators/mutation.h"
#include "../../../headers/global_modules/genetic_operators/crossover.h"

#include "../../../headers/global_modules/dominates.h"
#include "../../../headers/global_modules/isEqual.h"

#include "../../../headers/metaheuristics/moead/moead.h"

using namespace std;

void moead(vector<Solution>& population){

  random_device rd;
  mt19937 gen(rd());

  int size_population = population.size();
  double input_cross_prob = 0.5;
  double input_mutation_prob = 0.5;
  int number_of_neighbors = 10;

  vector<pair<double, double>> lambda_vector = build_weight_vector(size_population); 

  vector<vector<int>> neighborhood = build_neighborhood(number_of_neighbors, lambda_vector, size_population);
  
  pair<double, double> z_point = get_best_z_point(population);

  vector<double> tch_vector(size_population);

  for(int i = 0; i < tch_vector.size(); i++){
    tch_vector[i] = calculate_gte(population[i].fitness, lambda_vector[i], z_point);
  }
  
  int generation = 0;

  ofstream infoRun(root_folder + "infoRun.txt");

  while (countRevalue < stop_criteria) {

    infoRun << "Generation " << generation << " | Revalues: " << countRevalue << " | GridSize: " << pareto->getSize() << endl;

    for (int i = 0; i < size_population; i++) {

      uniform_int_distribution<> dis(0, number_of_neighbors - 1);
      int k = neighborhood[i][dis(gen)];
      int l = neighborhood[i][dis(gen)];

      while (k == l) {
        l = neighborhood[i][dis(gen)];
      }

      Solution * parentA = new Solution;
      Solution * parentB = new Solution;
      *parentA = population[k];
      *parentB = population[l];

      Solution * child1 = new Solution;
      Solution * child2 = new Solution;
      *child1 = *parentA;
      *child2 = *parentB;

      if((static_cast<double>(rand()) / RAND_MAX) < input_cross_prob){ 
        *child1 = crossover(*parentA, *parentB);
        *child2 = crossover(*parentB, *parentA);        
      }

      if((static_cast<double>(rand()) / RAND_MAX) < input_mutation_prob){
        mutation(*child1);
        mutation(*child2);
      }

      delete parentA;
      delete parentB;
      
      for(const auto sol : pareto->getElements()){
        z_point.first = max(z_point.first, sol->fitness.first);
        z_point.second = max(z_point.second, sol->fitness.second);
      }

      for (int j : neighborhood[i]) {
        double child1_tch = calculate_gte(child1->fitness, lambda_vector[j], z_point);
        double sol1_pop = calculate_gte(population[j].fitness, lambda_vector[j], z_point);
        if (child1_tch <= sol1_pop) {
          population[j] = *child1;
        }
        
        double child2_tch = calculate_gte(child2->fitness, lambda_vector[j], z_point);
        double sol2_pop = calculate_gte(population[j].fitness, lambda_vector[j], z_point);
        if (child2_tch <= sol2_pop) {
          population[j] = *child2;
        }
      }
      delete child1;
      delete child2;
    }
    generation++;
  }
  
  infoRun.close();

}
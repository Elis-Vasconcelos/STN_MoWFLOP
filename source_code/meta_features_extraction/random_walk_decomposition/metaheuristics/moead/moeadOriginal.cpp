#include <utility>
#include <vector>
#include <iostream>
#include <ctime> 
#include <algorithm>
#include <random>
#include "../../modules/headers/population.h"
#include "../../modules/headers/generate_rSolution.h"
#include "../../modules/headers/generate_weight_vectors.h"
#include "../../modules/headers/generate_neighborhood.h"
#include "../../modules/headers/get_best_z_point.h"
#include "../../modules/headers/tchebycheff.h"
#include "../../modules/headers/mutation.h"
#include "../../modules/headers/crossover.h"

#include "../../modules/headers/dominates.h"
#include "../../headers/utils/isEqual.h"

#include "../../modules/headers/updateEP.h"

using namespace std;

vector<Solution> moeadOriginal(vector<Solution>& population){

  
  default_random_engine re{(unsigned)time(nullptr)};
  uniform_real_distribution<double> dist(0.0, 1.0);

  
  int size_population = population.size();
  double input_cross_prob = 0.9;
  double input_mutation_prob = 0.1;
  int number_of_neighbors = 150;
  int max_generations = 500;

  
  
  vector<Solution> EP;
  for(auto& sol : population){
    updateEP(EP, sol);
  }

  
  vector<pair<double, double>> lambda_vector = build_weight_vector(size_population); 

  
  vector<vector<int>> neighborhood = build_neighborhood(number_of_neighbors, lambda_vector, size_population);
  
  
  pair<double, double> z_point = get_best_z_point(population);

  
  vector<double> tch_vector(size_population);

  for(int i = 0; i < tch_vector.size(); i++){
    tch_vector[i] = calculate_gte(population[i].fitness, lambda_vector[i], z_point);
  }
  
  cout << endl;

  

  int generation = 0;

  while (generation < max_generations) {

    Solution child1, child2;

    for (int i = 0; i < size_population; i++) {

      
      
      int k = neighborhood[i][rand() % neighborhood[i].size()];
      int l = neighborhood[i][rand() % neighborhood[i].size()];

      

      
      double random_cross_prob = dist(re);

      if(random_cross_prob < input_cross_prob){ 
        child1 = crossover(population[k], population[l]);
        child2 = crossover(population[l], population[k]);
      }
      else{
        child1 = population[k];
        child2 = population[l];
      }

      
      
      updateEP(EP, child1);
      updateEP(EP, child2);
      
      
      double random_mutation_prob = dist(re);

      if(random_mutation_prob < input_mutation_prob){
        mutation(child1);
        mutation(child2);
      }

      
      
      updateEP(EP, child1);
      updateEP(EP, child2);

      
      z_point.first = max(z_point.first, max(child1.fitness.first, child2.fitness.first));
      z_point.second = max(z_point.second, max(child1.fitness.second, child2.fitness.second));

      
      for (int j : neighborhood[i]) {
        double child1_tch = calculate_gte(child1.fitness, lambda_vector[j], z_point);
        if (child1_tch <= tch_vector[j]) {
          population[j] = child1;

          tch_vector[j] = child1_tch;
        }
        
        double child2_tch = calculate_gte(child2.fitness, lambda_vector[j], z_point);
        if (child2_tch <= tch_vector[j]) {
          population[j] = child2;
          tch_vector[j] = child2_tch;
        }
      }
    }
    generation++;
  }

  for(auto& i : EP){
    cout << i.fitness.first * (-1) << " " << i.fitness.second << endl;
  }

  return EP;
}
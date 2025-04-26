#include <iostream>
#include <ctime> 
#include <cstdlib>
#include <utility> 
#include <random>
#include "../../modules/headers/population.h"
#include "../../modules/headers/generate_rSolution.h"
#include "../../modules/headers/mutation.h"
#include "../../modules/headers/crossover.h"
#include "../../modules/headers/binary_tournament.h"
#include "../../modules/headers/crowding_distance.h"
#include "../../modules/headers/non_dominated_sorting.h"

#include "../../modules/headers/dominates.h"
#include "../../headers/utils/isEqual.h"
#include "../../modules/headers/nsga2.h"

using namespace std;

void updatePopulation(vector<Solution>& population) {
  for (int i = 0; i < population.size(); i++) {
    bool isDominated = false;

    for (int j = 0; j < population.size(); j++) {
      if (i != j && dominates(population[j], population[i])) {
        isDominated = true;
        population.erase(population.begin() + i);
        i--;
        break;
      }
    }

    if (!isDominated) {
      vector<int> copies;
      for (int j = 0; j < population.size(); j++) {
        if (i != j && isEqual(population[j], population[i])) {
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

vector<Solution> nsga2(vector<Solution>& population){

  default_random_engine re{(unsigned)time(nullptr)};
  uniform_real_distribution<double> dist(0.0, 1.0);

  int size_population = population.size(); 
  double cross_prob = 0.8;
  double mutation_prob = 0.4;
  int max_generations = 3;

  cout << endl;
  cout << "------------INITIAL POPULATION ------------ " << endl;
  for(auto& i : population){
    cout << "<" << i.fitness.first << ", " << i.fitness.second << ">" << endl;
  }
  cout << "------------------------------------------- " << endl << endl;

  cout << "SIZE OF INITIAL POPULATION: " << population.size() << endl << endl;

  int generation = 0;

  while(generation < max_generations){

    cout << "======================= GENERATION: " << generation << "=======================" << endl << endl;

    vector<Solution> offspring_population;

    for(int i = 0; i < size_population; i++){
      pair<Solution, Solution> parents = binary_tournament(population);
  
      Solution child1, child2;
      double k = dist(re);

      if(k < cross_prob){
        child1 = crossover(parents.first, parents.second);
        child2 = crossover(parents.second, parents.first);
      }
      else{
        child1 = parents.first;
        child2 = parents.second;
      }

      offspring_population.push_back(child1);
      offspring_population.push_back(child2);

      k = dist(re);

      if(k < mutation_prob){
        mutation(child1);
        mutation(child2);
      }

      offspring_population.push_back(child1);
      offspring_population.push_back(child2);
    }

    vector<Solution> total_population = population;
    total_population.insert(total_population.end(), offspring_population.begin(), offspring_population.end()); 
    cout << "SIZE OF TOTAL POPULATION: " << total_population.size() << endl << endl;

    vector<vector<Solution>> fronts = non_dominated_sorting(total_population);

    cout << "========================== FRONTS ==========================" << endl << endl;
    for(int i = 0; i < fronts.size(); i++){
      cout << "----------------------- FRONT: "<< i << " -----------------------" << endl;
      for(int j = 0; j < fronts[i].size(); j++){
        cout << "<" << fronts[i][j].fitness.first << ", " << fronts[i][j].fitness.second << ">"<< endl;
      }
    } cout << endl;

    population.clear(); 
    
    int k = 0;
    for (auto& front : fronts) {
      if (population.size() + front.size() <= size_population) {
        
        cout << "ADDING THE WHOLE FRONT " << k << endl;
        for (auto& solution : front) {
          population.push_back(solution);
        }
      } 
      else {
        
        auto front_sorted = crowding_distance(front);
        cout << "ADDING PART OF FRONT " << k << " (crowding distance)" << endl;
        
        
        int remaining_spots = size_population - population.size();
        
        
        for (int i = 0; i < remaining_spots; i++) {
          population.push_back(front_sorted[i]);
        }
        
        
        break;
      }
      k++;
    }

    generation++;
  }


  updatePopulation(population);
  
  cout << endl;
  cout << "------------FINAL POPULATION ------------ " << endl;
  for(auto& i : population){
    cout << "<" << i.fitness.first << ", " << i.fitness.second << ">" << endl;
  }
  cout << "------------------------------------------- " << endl << endl;

  cout << "SIZE OF FINAL POPULATION: " << population.size() << endl << endl;
  
  return population;
}
#include "../../headers/population.h"
#include "../../headers/crossover.h"
#include <cstdlib>
#include <vector>
#include <iostream>



extern vector<int> turbines_per_zone;


int find_turbine(vector<Turbine> &vec, int value){
  for(int i = 0; i < vec.size(); i++){
    if(vec[i].id == value){
      return i;
    }
  }
  return -1;
}



bool InOffspring(int turbine_index, const vector<int> &grid) {
  return grid[turbine_index] != 0;
}

void crossoverAux(Solution &parent_solutionA, Solution &parent_solutionB, Solution &offspring_solution, int zone){
  offspring_solution.grid[zone].resize(parent_solutionA.grid[zone].size(), 0);

  int rand_int = rand() % parent_solutionA.grid[zone].size();
  int index = -1;
  int lastIndex = 0;
  int id_new = fixd.size();
  Turbine turb;

  
  for(int i = 0; i <= rand_int; i++){
    if(parent_solutionA.grid[zone][i] != 0){
      offspring_solution.grid[zone][i] = id_new;
      index = find_turbine(parent_solutionA.turbines[zone], parent_solutionA.grid[zone][i]);
      turb = parent_solutionA.turbines[zone][index];
      turb.id = id_new;
      offspring_solution.turbines[zone].push_back(turb);
      id_new++;
    }

  }

  
  
  for(int i = parent_solutionB.grid[zone].size() - 1; i >= 0; i--){
    if(parent_solutionB.grid[zone][i] != 0){

      if(offspring_solution.turbines[zone].size() >= turbines_per_zone[zone]){
        return;
      }

      if(parent_solutionB.grid[zone][i] != 0){

        if(InOffspring(i, offspring_solution.grid[zone])){
          continue;
        }

        offspring_solution.grid[zone][i] = id_new;
        index = find_turbine(parent_solutionB.turbines[zone], parent_solutionB.grid[zone][i]);
        turb = parent_solutionB.turbines[zone][index];
        turb.id = id_new;
        offspring_solution.turbines[zone].push_back(turb);
        id_new++;
      }

    }
  }
}

Solution crossover(Solution &parent_solutionA, Solution &parent_solutionB){

  Solution offspring_solution;
  offspring_solution.grid.resize(num_zones);
  offspring_solution.turbines.resize(num_zones);

  for(int j = 0; j < num_zones; j++){
    crossoverAux(parent_solutionA, parent_solutionB, offspring_solution, j);
  }

  calculate_cost(offspring_solution);
  calculate_power(offspring_solution);

  return offspring_solution;
}
#include "../../headers/population.h"
#include "../../headers/mutation.h"
#include <algorithm>
#include <cstdlib>
using namespace std;


int find_index(vector<Turbine> &vec, int value){
  for(int i = 0; i < vec.size(); i++){
    if(vec[i].id == value){
      return i;
    }
  }

  return -1;
}

void mutationAux(Solution &solution, int zone){

    int zone_size = solution.grid[zone].size();
    int rand_intA, rand_intB;
    int indexA = -1;
    int indexB = -1;

    do{
      rand_intA = rand() % zone_size;
      rand_intB = rand() % zone_size;
    } while(rand_intA == rand_intB || (solution.grid[zone][rand_intA] == 0 && solution.grid[zone][rand_intB] == 0));

    int geneA = solution.grid[zone][rand_intA];
    int geneB = solution.grid[zone][rand_intB];

    if(solution.grid[zone][rand_intA] != 0){
      indexA = find_index(solution.turbines[zone], solution.grid[zone][rand_intA]);
    }

    if(solution.grid[zone][rand_intB] != 0){
      indexB = find_index(solution.turbines[zone], solution.grid[zone][rand_intB]);
    }

    if(indexA != -1 && indexB != -1){
      int index;
      double x, y;

      index = solution.turbines[zone][indexA].index;
      x = solution.turbines[zone][indexA].x;
      y = solution.turbines[zone][indexA].y;

      solution.turbines[zone][indexA].index = solution.turbines[zone][indexB].index;
      solution.turbines[zone][indexA].x = solution.turbines[zone][indexB].x;
      solution.turbines[zone][indexA].y = solution.turbines[zone][indexB].y;

      solution.turbines[zone][indexB].index = index;
      solution.turbines[zone][indexB].x = x;
      solution.turbines[zone][indexB].y = y;
    } else if (indexA != -1 && indexB == -1){
      solution.turbines[zone][indexA].index = rand_intB;
      solution.turbines[zone][indexA].x = foundations[zone][rand_intB].x;
      solution.turbines[zone][indexA].y = foundations[zone][rand_intB].y;
    } else {
      solution.turbines[zone][indexB].index = rand_intA;
      solution.turbines[zone][indexB].x = foundations[zone][rand_intA].x;
      solution.turbines[zone][indexB].y = foundations[zone][rand_intA].y;
    }

    solution.grid[zone][rand_intA] = geneB;
    solution.grid[zone][rand_intB] = geneA;
}

void mutation(Solution &solution){
  for(int i = 0; i < num_zones; i++){
    mutationAux(solution, i);
  }

  calculate_cost(solution);
  calculate_power(solution);
}
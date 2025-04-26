#include "../../headers/population.h"
#include "../../headers/mutation2.h"
#include "../../headers/updateEP.h"

#include <algorithm>
#include <random>
#include <ctime> 
#include <cstdlib>
using namespace std;



void mutationAux2(Solution &solution, int zone, double input_mutation_prob, vector<Solution> &EP){
    random_device rd;
    mt19937 gen(rd());

    int zone_size = solution.turbines[zone].size();
    int rand_intB;
    int indexA = -1;
    int indexB = -1;

    for(int i = 0; i < zone_size; ++i){
      if((static_cast<double>(rand()) / RAND_MAX) < input_mutation_prob){
        do {
          rand_intB = rand() % solution.grid[zone].size();
        } while(solution.grid[zone][rand_intB] != 0);  
  
        int geneA = solution.turbines[zone][i].index;
        int geneB = 0;

        solution.turbines[zone][i].index = rand_intB;
        solution.turbines[zone][i].x = foundations[zone][rand_intB].x;
        solution.turbines[zone][i].y = foundations[zone][rand_intB].y;

        solution.grid[zone][geneA] = 0;
        solution.grid[zone][rand_intB] = solution.turbines[zone][i].id;

        updateEP(EP, solution);
      }
    }
}

void mutation2(Solution &solution, double input_mutation_prob, vector<Solution> &EP){
  for(int i = 0; i < num_zones; i++){
    mutationAux2(solution, i, input_mutation_prob, EP);
  }

  calculate_cost(solution);
  calculate_power(solution);
}
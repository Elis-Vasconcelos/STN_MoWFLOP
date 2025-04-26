#include <vector>
#include <utility>
#include <iostream>
#include "../../../../headers/metaheuristics/nsga2/modules/non_dominated_sorting.h"
#include "../../../../headers/global_modules/dominates.h"

vector<vector<Solution*>*> non_dominated_sorting(vector<Solution*>& population_rt){

  vector<Solution*> solutions = population_rt;


  vector<vector<int>> domination_vector(solutions.size()); 
  vector<int> domination_count_vector(solutions.size(), 0); 

  vector<vector<int>> fronts_indices;
  fronts_indices.push_back(*(new vector<int>)); 
  vector<vector<Solution*>*> fronts;
  fronts.push_back(new vector<Solution*>());

  for(int p = 0; p < solutions.size(); p++){
    int domination_count = 0;
    for(int q = 0; q < solutions.size(); q++){
      if(p != q){
        if(dominates(*solutions[p], *solutions[q])){
          domination_vector[p].push_back(q);
        }
        else if(dominates(*solutions[q], *solutions[p])){
          domination_count++;
        }
      }
    }

    domination_count_vector[p] = domination_count;

    if(domination_count == 0){
      fronts[0]->push_back(new Solution(*solutions[p]));
      fronts_indices[0].push_back(p);
    }
  }

  int i = 0; 
  while(!fronts[i]->empty()){
    vector<Solution*> * next_front = new vector<Solution*>();
    vector<int> next_front_indices;

    for (int p_index : fronts_indices[i]) {
      for (int q_index : domination_vector[p_index]) {
        domination_count_vector[q_index]--;

        if (domination_count_vector[q_index] == 0) {
          next_front->push_back(new Solution(*solutions[q_index]));
          next_front_indices.push_back(q_index);
        }
      }
    }
    fronts.push_back(next_front);
    fronts_indices.push_back(next_front_indices);
    i++;
  }

  if (fronts.back()->empty()) {
    fronts.pop_back();
  }

  return fronts;
}
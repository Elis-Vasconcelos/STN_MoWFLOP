#include <vector>
#include <utility>
#include <iostream>
#include "../headers/non_dominated_sorting.h"
#include "../headers/dominates.h"


bool dominates(Solution solutionA, Solution solutionB){
  return (solutionA.fitness.first >= solutionB.fitness.first && solutionA.fitness.second >= solutionB.fitness.second) && (solutionA.fitness.first > solutionB.fitness.first || solutionA.fitness.second > solutionB.fitness.second);
}


vector<vector<Solution>> non_dominated_sorting(vector<Solution>& solutions){
  vector<vector<int>> fronts_indexes; 
  vector<int> first_front; 

  

  vector<vector<int>> domination_vector(solutions.size()); 
  vector<int> domination_count_vector(solutions.size()); 

  
  for(int p = 0; p < solutions.size(); p++){
    int domination_count = 0;
    for(int q = 0; q < solutions.size(); q++){
      if(p != q){
        if(dominates(solutions[p], solutions[q])){
          domination_vector[p].push_back(q); 
        }
        else if(dominates(solutions[q], solutions[p])){
          domination_count++;
        }
      }
    }

    domination_count_vector[p] = domination_count;

    if(domination_count == 0){
      first_front.push_back(p);
    }
  }

  fronts_indexes.push_back(first_front);
  
  int i = 0; 
  while(!fronts_indexes[i].empty()){
    vector<int> next_front;

    for (int p_index : fronts_indexes[i]) {
      for (int q_index : domination_vector[p_index]) {
        
        domination_count_vector[q_index]--;

        if (domination_count_vector[q_index] == 0) {
          next_front.push_back(q_index);
        }
      }
    }
    i++;
    fronts_indexes.push_back(next_front);
  }

  if (fronts_indexes.back().empty()) {
    fronts_indexes.pop_back();
  }

  
  vector<vector<Solution>> fronts;
  for (const auto& front : fronts_indexes) {
    vector<Solution> front_solutions;
    for (int index : front) {
      front_solutions.push_back(solutions[index]);
    }
    fronts.push_back(front_solutions);
  }

  return fronts;
}
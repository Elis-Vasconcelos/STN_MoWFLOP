#include "../headers/generate_rSolution.h"
#include "../headers/dominates.h"
#include "../../headers/utils/isEqual.h"
#include "../headers/updateEP.h"

void updateEP(vector<Solution> &EP, Solution &child){
  
  
  
  
  EP.erase(remove_if(EP.begin(), EP.end(), [&](const Solution &sol) {
    return dominates(child, sol);
  }), EP.end());

  
  bool is_dominated = false;
  bool already_exists = false;

  for (const auto& sol : EP) {
    if (dominates(sol, child)) {
      is_dominated = true;
      break;
    }
    if (isEqual(sol, child)) {
      already_exists = true;
      break;
    }
  }  

  if (!is_dominated && !already_exists) {
    EP.push_back(child);
  }
}
#ifndef LANDSCAPE_ELEMENT_H
#define LANDSCAPE_ELEMENT_H

#include <vector>
#include "../../modules/headers/generate_rSolution.h"

struct LandscapeElement {
  Solution current_solution;
  double tch_current_solution;
  double inf; 
  double sup; 
  double inc; 
  double ind; 
  vector<double> tchebycheff_neighbors;
};

#endif 

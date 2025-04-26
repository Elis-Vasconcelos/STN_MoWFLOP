#include <vector>
#include <iostream>

#include "../../headers/metafeatures/landscapeElement.h"
#include "../../headers/metafeatures/normalization.h"

void normalization(vector<vector<LandscapeElement>>& landscapes, double &max, double &min){

  for(auto& landscape : landscapes){
    for(auto& element : landscape){
      element.tch_current_solution = (element.tch_current_solution - min) / (max - min);

      for(auto& neighbor : element.tchebycheff_neighbors){
        neighbor = (neighbor - min) / (max - min);
      }
    }
  }

}
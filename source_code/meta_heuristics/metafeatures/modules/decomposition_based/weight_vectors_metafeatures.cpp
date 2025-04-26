#include <utility>
#include <vector>
#include <iostream>
#include "../../../headers/metafeatures/decomposition_based/weight_vectors_metafeatures.h"

using namespace std;

vector<pair<double, double>> build_weight_vector_metafeatures(int population_size){
  vector<pair<double, double>> weight_vectors;

  if(population_size == 1){
    weight_vectors.push_back(make_pair(0.5, 0.5));
  }
  else{
    double step = 1.0 / (population_size - 1); 
    for (int i = 0; i < population_size; ++i) {
      double w1 = step * i; 
      double w2 = 1.0 - w1; 
      weight_vectors.push_back(make_pair(w1, w2)); 
    }
  } 

  return weight_vectors; 
}
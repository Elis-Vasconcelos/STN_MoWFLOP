#include <vector>
#include <utility>
#include <iostream>
#include <cmath>
#include <algorithm> 
#include "../../../../headers/metaheuristics/moead/modules/generate_neighborhood.h"
using namespace std;

double euclidean_distance(pair<double, double>& a, pair<double, double>& b) {
  return sqrt(pow(a.first - b.first, 2) + pow(a.second - b.second, 2));
}

vector<vector<int>> build_neighborhood(int number_of_neighbors, vector<pair<double, double>>& lambda_vector, int size_population){

  vector<vector<int>> neighborhood(size_population);

  if (number_of_neighbors > size_population - 1) {
    throw runtime_error("ERROR - MOEAD: generate_neighbood ==> The number of neighbors cannot be greater than the population size minus 1.");
  }

  for (int i = 0; i < lambda_vector.size(); i++) {
    vector<pair<double, int>> distances; 

    for (int j = 0; j < lambda_vector.size(); j++) {
      if (i != j) {  
        double distance = euclidean_distance(lambda_vector[i], lambda_vector[j]);
        distances.push_back(make_pair(distance, j));
      }
    }

    sort(distances.begin(), distances.end());

    for (int t = 0; t < number_of_neighbors; t++) {
      neighborhood[i].push_back(distances[t].second); 
    }
  }  

  return neighborhood;
}
#include <vector>
#include <cmath>
#include <iostream>
#include <utility>
#include <limits> 
#include "../../headers/metafeatures/decomposition_based/tchebycheff_metafeatures.h"

using namespace std;


double calculate_gte_metafeatures(pair<double, double>& x_j, pair<double, double>& lambda_j, pair<double, double>& z_point){

  vector<double> f_values = {x_j.first, x_j.second};

  vector<double> lambda_j_values = {lambda_j.first, lambda_j.second};

  vector<double> z_values = {z_point.first, z_point.second};  

  double max_value = numeric_limits<double>::lowest();
  for (int i = 0; i < f_values.size(); ++i) {
    double current_value = lambda_j_values[i] * abs(f_values[i] - z_values[i]);
    if (current_value > max_value) {
      max_value = current_value;
    }
  }

  return max_value;
}

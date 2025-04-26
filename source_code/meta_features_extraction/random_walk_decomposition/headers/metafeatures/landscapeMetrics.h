#ifndef LANDSCAPE_METRICS_H
#define LANDSCAPE_METRICS_H

#include <vector>

struct LandscapeMetrics {
  std::vector<double> fitness_values; 
  std::vector<double> fitness_differences; 
  std::vector<double> improving_neighbors_count; 
  std::vector<double> inf_neighbors; 
  std::vector<double> sup_neighbors; 
  std::vector<double> inc_neighbors; 
  std::vector<double> ind_neighbors; 
};

#endif 
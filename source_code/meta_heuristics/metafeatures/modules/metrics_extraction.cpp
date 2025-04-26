#include <vector>
#include <utility>

#include "../../headers/metafeatures/landscapeElement.h"
#include "../../headers/metafeatures/landscapeMetrics.h"
#include "../../headers/metafeatures/metrics_extraction.h"

LandscapeMetrics metric_extraction(vector<LandscapeElement> &landscape){
  LandscapeMetrics landscape_metrics;

  for(const auto& element : landscape){ 
    double current_solution_fitness = element.tch_current_solution;
    int improving_count = 0;
    double total_difference = 0.0;

    landscape_metrics.fitness_values.push_back(current_solution_fitness);

    for(const auto& neighbor_fitness: element.tchebycheff_neighbors){
      if(neighbor_fitness < current_solution_fitness){
        improving_count++; 
      }

      double difference = current_solution_fitness - neighbor_fitness;
      total_difference += difference;
    }

    double avg_fitness_difference = total_difference / element.tchebycheff_neighbors.size();
    landscape_metrics.fitness_differences.push_back(avg_fitness_difference);
    
    double normalized_improving_neighbors = static_cast<double>(improving_count);
    if(improving_count > 0){
      normalized_improving_neighbors = static_cast<double>(improving_count) / element.tchebycheff_neighbors.size();
    }

    landscape_metrics.inf_neighbors.push_back(element.inf);

    landscape_metrics.sup_neighbors.push_back(element.sup);

    landscape_metrics.inc_neighbors.push_back(element.inc);

    landscape_metrics.ind_neighbors.push_back(element.ind);

    landscape_metrics.improving_neighbors_count.push_back(normalized_improving_neighbors);
  }

    return landscape_metrics;
}

vector<LandscapeMetrics> metrics_extraction(vector<vector<LandscapeElement>> &landscapes){

  vector<LandscapeMetrics> landscape_so_features;

  for(int i = 0; i < landscapes.size(); i++){
    LandscapeMetrics metric = metric_extraction(landscapes[i]);
    landscape_so_features.push_back(metric);
  }

  return landscape_so_features;
}
#include <vector>
#include <iostream>
#include <numeric>
#include <cmath>
#include <string>
#include <functional>

#include "../../headers/metafeatures/landscapeMetrics.h"
#include "../../headers/metafeatures/decomposition_based/mo_features_decomposition.h"
#include "../../headers/metafeatures/statistical_functions.h"

using namespace std;

vector<double> get_mo_features(vector<LandscapeMetrics> &landscapes_vector, function<vector<double>(const LandscapeMetrics&)> get_feature){
  vector<double> mo_features;
  vector<double> s_mean;
  vector<double> s_sd;
  vector<double> s_r1;
  vector<double> s_kr;
  vector<double> s_sk;

  
  for(auto& landscape : landscapes_vector){
    vector<double> feature = get_feature(landscape);
    s_mean.push_back(mean(feature)); 
    s_sd.push_back(sd(feature)); 
    s_r1.push_back(r1(feature)); 
    if(s_sd.back() != 0){
      s_kr.push_back(kr(feature)); 
      s_sk.push_back(sk(feature)); 
    }
  }

  

  
  mo_features.push_back(mean(s_mean)); 
  mo_features.push_back(sd(s_mean)); 

  
  mo_features.push_back(mean(s_sd)); 
  mo_features.push_back(sd(s_sd)); 

  
  mo_features.push_back(mean(s_r1)); 
  mo_features.push_back(sd(s_r1)); 

  
  mo_features.push_back(mean(s_kr)); 
  mo_features.push_back(sd(s_kr)); 

  
  mo_features.push_back(mean(s_sk)); 
  mo_features.push_back(sd(s_sk)); 

  return mo_features;
}

vector<double> mo_features_extraction(vector<LandscapeMetrics> landscapes_vector){
  vector<double> mo_features;
  vector<double> fitness_values;
  vector<double> fitness_differences;
  vector<double> improving_neighbors;

  fitness_values = get_mo_features(landscapes_vector, [](const LandscapeMetrics& landscape) { return landscape.fitness_values;});
  fitness_differences = get_mo_features(landscapes_vector, [](const LandscapeMetrics& landscape) { return landscape.fitness_differences;});
  improving_neighbors = get_mo_features(landscapes_vector, [](const LandscapeMetrics& landscape) { return landscape.improving_neighbors_count;});

  mo_features = fitness_values;
  mo_features.insert(mo_features.end(), fitness_differences.begin(), fitness_differences.end());
  mo_features.insert(mo_features.end(), improving_neighbors.begin(), improving_neighbors.end());

  return mo_features;
}
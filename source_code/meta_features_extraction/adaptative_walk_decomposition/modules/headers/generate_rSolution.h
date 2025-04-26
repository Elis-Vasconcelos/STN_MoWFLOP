#ifndef GENERATE_RSOLUTION_H
#define GENERATE_RSOLUTION_H

#include <vector>
#include <utility>
#include "turbine.h"
using namespace std;

struct Foundation {
    double x;
    double y;
    double cost;
};

float power_alt(float& wind, float& power, float& tc);

extern vector<vector<Foundation>> foundations;
extern vector<Turbine> fixd;
extern vector<int> turbines_per_zone;

extern float wind;
extern float power;
extern float thrust_coef;
extern float angle;
extern int num_zones;
extern int id;

struct Solution {
    vector<vector<int>> grid;
    vector<vector<Turbine>> turbines; 
    pair<double, double> fitness;  
};

Solution generate_solution();

double calculate_cost(Solution& sol);

double calculate_interference(Turbine& t_initial, Turbine& t_interfered);

double calculate_power(Solution& sol);

#endif
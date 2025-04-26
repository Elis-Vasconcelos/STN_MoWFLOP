#ifndef SOLUTION_H
#define SOLUTION_H

#include <vector>
#include <utility>
#include "./turbine.h"

struct Solution {
    std::vector<std::vector<int>> grid;
    std::vector<std::vector<Turbine>> turbines;
    std::pair<double, double> fitness;
};

#endif 
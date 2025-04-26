#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "../../headers/global_modules/generate_initial_population/generate_rSolution.h"
#include <iomanip>

using namespace std;

bool dominates(const Solution& sol1, const Solution& sol2) {
    return (sol1.fitness.first < sol2.fitness.first && sol1.fitness.second > sol2.fitness.second);
}

void compareSolutions(const string& fileA, const string& fileB) {
    ifstream inFileA(fileA);
    ifstream inFileB(fileB);
    
    if (!inFileA || !inFileB) {
        cerr << "File Opening Error." << endl;
        return;
    }

    vector<Solution> solutionsA;
    vector<Solution> solutionsB;

    double x, y;
    while (inFileA >> x >> y) {
        Solution sol;
        sol.fitness.first = x;
        sol.fitness.second = y;
        solutionsA.push_back(sol);
        cout << "Reading Solution on file A: " << x << ", " << y << endl;
    }

    while (inFileB >> x >> y) {
        Solution sol;
        sol.fitness.first = x;
        sol.fitness.second = y;
        solutionsB.push_back(sol);
        cout << "Reading Solution on file A: " << x << ", " << y << endl;
    }

    inFileA.close();
    inFileB.close();

    for (const auto& solA : solutionsA) {
        int count = 0; 
        for (const auto& solB : solutionsB) {
            if (dominates(solA, solB)) {
                count++;
            }
        }
        cout << "Solution " << solA.fitness.first << ", " << solA.fitness.second 
             << " dominates " << count << " solutions from file B." << endl;
    }
}

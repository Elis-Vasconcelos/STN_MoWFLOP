#include "../../modules/headers/generate_rSolution.h"
#include <iostream>
using namespace std;

bool isValid(Solution &solution){

  for(int zone = 0; zone < num_zones; zone++){


    

    for(int i = 0; i < solution.turbines[zone].size(); i++){
      int index_foundation = solution.turbines[zone][i].index;
      int turbine_id_grid = solution.grid[zone][index_foundation];
      int turbine_id = solution.turbines[zone][i].id;
      if(!(turbine_id_grid == turbine_id)){
        cout << turbine_id << " " << turbine_id_grid << endl;
        cout << "Grid differs from turbine position" << endl;
        return false;
      }
    }

    
    for(int i = 0; i < solution.turbines[zone].size(); i++){
      for(int j = 0; j < solution.turbines[zone].size(); j++){
        if(i != j){
          if(solution.turbines[zone][i].index == solution.turbines[zone][j].index){
              cout << solution.turbines[zone][i].id << " " << solution.turbines[zone][j].id  << " " << solution.turbines[zone][i].x << " " << solution.turbines[zone][j].x << " " <<
              solution.turbines[zone][i].y << " " << solution.turbines[zone][j].y << endl;
              cout << solution.turbines[zone][i].index << " " << solution.turbines[zone][j].index << endl;
              cout << "Two turbines on same position" << endl;
            return false;
          }
        }
      }
    }

    

    for(int i = 0; i < solution.turbines[zone].size(); i++){
      if(!(foundations[zone][solution.turbines[zone][i].index].x == solution.turbines[zone][i].x) ||
         !(foundations[zone][solution.turbines[zone][i].index].y == solution.turbines[zone][i].y)
      ) {
        cout << "Index isn't on correct position" << endl;
        return false;
      }
    }

  }

  return true;
}
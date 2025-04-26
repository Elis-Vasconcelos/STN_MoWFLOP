#include <vector>
#include <limits>
#include <iostream>
#include <utility>
#include "../../headers/metafeatures/decomposition_based/global_z_point.h"
#include "../../modules/headers/generate_rSolution.h"

using namespace std;

pair<double, double> get_global_z_point() {
  
  double power_turb_changeable = power_alt(wind, power, thrust_coef);
  
  double power_turb_fixed = power_alt(wind, powerFxd, tcFxd);

  double max_power_fixed = power_turb_fixed * fixd.size();


  int num_turb = 0;

  for (int i = 0; i < turbines_per_zone.size(); i++){
    num_turb += turbines_per_zone[i];
  }

  double max_power_changeable = power_turb_changeable * num_turb;

  double best_power = max_power_fixed + max_power_changeable;

  return make_pair(0, best_power);
}

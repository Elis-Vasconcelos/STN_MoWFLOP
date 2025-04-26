#include "../../../../headers/global_modules/generate_initial_population/generate_rSolution.h"
#include "../../../../headers/global_modules/dominates.h"
#include "../../../../headers/global_modules/isEqual.h"
#include "../../../../headers/adaptative_grid/ParetoSet.h"

#include "../../../../headers/metaheuristics/moead/modules/updateEP.h"

void updateEP(list<Solution *> &EP, Solution *child){
  list<Solution *>::iterator i = EP.begin();
  list< list<Solution *>::iterator > toRemove;
  while (i != EP.end()) {
    if (dominates(*child, **i)) {
      toRemove.push_back(i);
    }
    if (dominates(**i, *child)|| equals(**i, *child)){
      return;
    }
    i++;
	}


  list< list<Solution *>::iterator >::iterator j = toRemove.begin();
	while (j != toRemove.end()) {
		delete( **j );
		
    EP.erase(*j);
		j++;
	}
  
    Solution *t = new Solution;
    *t = *child;
    EP.push_back(t);
}
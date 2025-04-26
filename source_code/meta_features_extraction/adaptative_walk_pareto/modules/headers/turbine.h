#ifndef TURBINE_H
#define TURBINE_H

using namespace std;

struct Turbine {
    int id;
    int index;
    double x;
    double y;
    float power;
    float thrust_coef;
    int diameter;
    int zone;
    int height;
};




float power_produced(double& wind, Turbine& turbine);

#endif 